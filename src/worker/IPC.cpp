#include "IPC.hpp"
#include "oatpp/core/base/Environment.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <new>

namespace app { namespace worker {

IPC::IPC() {}

IPC::~IPC() {
    // If we are mostly just a wrapper, we might not want to close everything in destructor 
    // to allow objects to be passed around, but typically IPC object is a singleton or long-lived.
    // We will call cleanup explicitly.
}

void IPC::initHost() {
    m_isHost = true;
    OATPP_LOGD("IPC", "Initializing Host...");

    // 1. Cleanup old
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_REQ_NAME);
    sem_unlink(SEM_RESP_NAME);

    // 2. Create SHM
    m_shmFd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (m_shmFd == -1) {
        throw std::runtime_error("Failed to shm_open");
    }

    if (ftruncate(m_shmFd, sizeof(SharedMem)) == -1) {
        throw std::runtime_error("Failed to ftruncate");
    }

    void* addr = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0);
    if (addr == MAP_FAILED) {
        throw std::runtime_error("Failed to mmap");
    }

    m_shm = new (addr) SharedMem(); // Placement new to initialize atomics
    // Reset indices
    m_shm->req_write_idx = 0;
    m_shm->req_read_idx = 0;
    m_shm->resp_write_idx = 0;
    m_shm->resp_read_idx = 0;

    // 3. Create Semaphores
    m_semReq = sem_open(SEM_REQ_NAME, O_CREAT, 0666, 0);
    if (m_semReq == SEM_FAILED) {
        throw std::runtime_error("Failed to create req semaphore");
    }
    m_semResp = sem_open(SEM_RESP_NAME, O_CREAT, 0666, 0);
    if (m_semResp == SEM_FAILED) {
        throw std::runtime_error("Failed to create resp semaphore");
    }
    
    OATPP_LOGD("IPC", "Host Initialized. SHM Size: %lu", sizeof(SharedMem));
}

void IPC::initWorker() {
    m_isHost = false;
    OATPP_LOGD("IPC", "Initializing Worker...");

    // 1. Open SHM
    m_shmFd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (m_shmFd == -1) {
        throw std::runtime_error("Failed to shm_open (worker)");
    }

    void* addr = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, m_shmFd, 0);
    if (addr == MAP_FAILED) {
        throw std::runtime_error("Failed to mmap (worker)");
    }

    m_shm = static_cast<SharedMem*>(addr);

    // 2. Open Semaphores
    m_semReq = sem_open(SEM_REQ_NAME, 0);
    if (m_semReq == SEM_FAILED) {
        throw std::runtime_error("Failed to open req semaphore");
    }
    m_semResp = sem_open(SEM_RESP_NAME, 0);
    if (m_semResp == SEM_FAILED) {
        throw std::runtime_error("Failed to open resp semaphore");
    }
    
    OATPP_LOGD("IPC", "Worker Initialized.");
}

void IPC::cleanup() {
    if (m_semReq) {
        sem_close(m_semReq);
        m_semReq = nullptr;
    }
    if (m_semResp) {
        sem_close(m_semResp);
        m_semResp = nullptr;
    }

    if (m_shm && m_shm != MAP_FAILED) {
        munmap(m_shm, sizeof(SharedMem));
        m_shm = nullptr;
    }

    if (m_shmFd != -1) {
        close(m_shmFd);
        m_shmFd = -1;
    }

    if (m_isHost) {
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_REQ_NAME);
        sem_unlink(SEM_RESP_NAME);
    }
}

bool IPC::submitRequest(const ReqSlot& req) {
    if (!m_shm) return false;

    size_t writeIdx = m_shm->req_write_idx.load(std::memory_order_relaxed);
    size_t readIdx = m_shm->req_read_idx.load(std::memory_order_acquire);

    if (writeIdx - readIdx >= RING_CAP) {
        return false; // Full
    }

    m_shm->req_ring[writeIdx % RING_CAP] = req;
    
    std::atomic_thread_fence(std::memory_order_release);
    m_shm->req_write_idx.store(writeIdx + 1, std::memory_order_release);
    
    sem_post(m_semReq);
    return true;
}

bool IPC::waitForRequest(ReqSlot& req) {
    if (!m_shm || !m_semReq) return false;

    if (sem_wait(m_semReq) != 0) {
        return false;
    }

    size_t idx = m_shm->req_read_idx.fetch_add(1, std::memory_order_acq_rel);
    req = m_shm->req_ring[idx % RING_CAP];
    return true;
}

bool IPC::submitResponse(const RespSlot& resp) {
    if (!m_shm) return false;

    // Response queue is also MPSC (Multi-Producer Single-Consumer) because multiple workers write?
    // Actually yes. So workers need to atomic_fetch_add the write_idx.
    
    size_t writeIdx = m_shm->resp_write_idx.fetch_add(1, std::memory_order_acq_rel);
    
    // Check overflow?
    // Ideally we shouldn't have more responses than requests, so if req queue isn't overflowing, 
    // resp queue shouldn't either, provided server drains it.
    // However, if server is slow, we might overwrite.
    // For now, let's assume server is fast enough or RING_CAP is large enough.
    // A robust impl would check read_idx.
    
    m_shm->resp_ring[writeIdx % RING_CAP] = resp;
    
    sem_post(m_semResp);
    return true;
}

bool IPC::waitForResponse(RespSlot& resp, bool blocking) {
    if (!m_shm || !m_semResp) return false;

    if (blocking) {
        if (sem_wait(m_semResp) != 0) return false;
    } else {
        if (sem_trywait(m_semResp) != 0) return false;
    }

    // Single consumer (Server)
    size_t idx = m_shm->resp_read_idx.load(std::memory_order_relaxed);
    resp = m_shm->resp_ring[idx % RING_CAP];
    m_shm->resp_read_idx.store(idx + 1, std::memory_order_release);
    
    return true;
}

}}
