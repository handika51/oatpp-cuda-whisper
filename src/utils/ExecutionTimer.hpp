#ifndef Utils_ExecutionTimer_hpp
#define Utils_ExecutionTimer_hpp

#include <chrono>

namespace app { namespace utils {

class ExecutionTimer {
private:
    std::chrono::high_resolution_clock::time_point m_start;
public:
    ExecutionTimer() {
        m_start = std::chrono::high_resolution_clock::now();
    }

    long long getElapsedMicros() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
    }
};

}}

#endif