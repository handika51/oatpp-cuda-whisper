#include "WorkerManager.hpp"
#include <iostream>
#include <csignal>
#include <sys/wait.h>
#include <chrono>

namespace app { namespace worker {

WorkerManager::WorkerManager() {}

WorkerManager::~WorkerManager() {
    stop();
}

void WorkerManager::start(int numWorkers, const char* execPath) {
    if (m_running) return;

    m_ipc.initHost();
    m_running = true;

    // Start Response Thread
    m_responseThread = std::thread(&WorkerManager::responseLoop, this);

    std::cout << "Starting " << numWorkers << " workers..." << std::endl;

    // Fork Workers
    for (int i = 0; i < numWorkers; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child
            // Re-execute self with --worker flag
            // Ensure execPath is valid.
            execl(execPath, execPath, "--worker", (char*)NULL);
            // If exec fails
            std::cerr << "Failed to execl worker!" << std::endl;
            exit(1);
        } else if (pid > 0) {
            m_workerPids.push_back(pid);
        } else {
            std::cerr << "Failed to fork worker" << std::endl;
        }
    }
}

void WorkerManager::stop() {
    if (!m_running) return;
    m_running = false;

    // Send Shutdown Signal via SHM
    for (size_t i = 0; i < m_workerPids.size(); ++i) {
        sendShutdownSignal();
    }

    // Detach response thread (simplest shutdown for blocking sem_wait)
    if (m_responseThread.joinable()) {
        m_responseThread.detach(); 
    }

    // Wait for children
    for (pid_t pid : m_workerPids) {
        int status;
        waitpid(pid, &status, 0);
    }
    m_workerPids.clear();

    m_ipc.cleanup();
}

void WorkerManager::sendShutdownSignal() {
    ReqSlot req;
    req.task_id = 0;
    req.type = TASK_SHUTDOWN;
    m_ipc.submitRequest(req);
}

void WorkerManager::responseLoop() {
    while (m_running) {
        RespSlot resp;
        // Blocking wait
        if (m_ipc.waitForResponse(resp, true)) {
            std::lock_guard<std::mutex> lock(m_mapMutex);
            auto it = m_pendingTasks.find(resp.task_id);
            if (it != m_pendingTasks.end()) {
                it->second.set_value(resp);
                m_pendingTasks.erase(it);
            } else {
                // Unknown task or timed out?
            }
        } else {
            // Error
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

std::future<RespSlot> WorkerManager::submitTask(const ReqSlot& req) {
    ReqSlot mutableReq = req;
    mutableReq.task_id = m_taskIdCounter++;
    mutableReq.enqueue_timestamp_ns = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    std::promise<RespSlot> promise;
    auto future = promise.get_future();

    {
        std::lock_guard<std::mutex> lock(m_mapMutex);
        m_pendingTasks[mutableReq.task_id] = std::move(promise);
    }

    if (!m_ipc.submitRequest(mutableReq)) {
        // Queue full - cleanup and throw
        {
            std::lock_guard<std::mutex> lock(m_mapMutex);
            m_pendingTasks.erase(mutableReq.task_id);
        }
        throw std::runtime_error("Request Queue Full");
    }

    return future;
}

}}
