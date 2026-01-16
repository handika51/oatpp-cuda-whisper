#ifndef WORKER_IPC_HPP
#define WORKER_IPC_HPP

#include "SharedMemoryStructs.hpp"
#include <string>
#include <semaphore.h>
#include <optional>

namespace app { namespace worker {

class IPC {
private:
    int m_shmFd = -1;
    SharedMem* m_shm = nullptr;
    sem_t* m_semReq = nullptr;
    sem_t* m_semResp = nullptr;
    bool m_isHost = false;

public:
    IPC();
    ~IPC();

    // Initialize as the Host (Server). Creates SHM and Semaphores.
    void initHost();

    // Initialize as a Worker. Attaches to existing SHM and Semaphores.
    void initWorker();

    void cleanup();

    // --- Request Queue Operations ---
    
    // For Host to send work
    bool submitRequest(const ReqSlot& req);
    
    // For Worker to get work (blocking or non-blocking)
    // Returns true if a request was retrieved
    bool waitForRequest(ReqSlot& req);

    // --- Response Queue Operations ---

    // For Worker to send result
    bool submitResponse(const RespSlot& resp);

    // For Host to get result
    // Returns true if a response was retrieved
    bool waitForResponse(RespSlot& resp, bool blocking = true);

    SharedMem* getMemory() const { return m_shm; }
};

}}

#endif
