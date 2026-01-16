#ifndef WORKER_MANAGER_HPP
#define WORKER_MANAGER_HPP

#include "IPC.hpp"
#include <thread>
#include <mutex>
#include <map>
#include <future>
#include <atomic>
#include <vector>
#include <unistd.h>

namespace app { namespace worker {

class WorkerManager {
private:
    IPC m_ipc;
    std::thread m_responseThread;
    std::atomic<bool> m_running{false};
    std::atomic<uint64_t> m_taskIdCounter{1};

    std::mutex m_mapMutex;
    std::map<uint64_t, std::promise<RespSlot>> m_pendingTasks;

    std::vector<pid_t> m_workerPids;

    void responseLoop();

public:
    WorkerManager();
    ~WorkerManager();

    // Start workers. execPath is the path to the current executable.
    void start(int numWorkers, const char* execPath);
    void stop();
    
    // Send a single shutdown signal (useful for manual/test workers)
    void sendShutdownSignal();

    std::future<RespSlot> submitTask(const ReqSlot& req);
};

}}

#endif
