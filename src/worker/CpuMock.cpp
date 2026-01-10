#include "Bridge.hpp"
#include "oatpp/core/base/Environment.hpp"
#include <thread>
#include <chrono>

namespace app { namespace worker {

void AudioWorker::computeMelSpectrogram(const std::vector<float>& inputAudio, std::vector<float>& outputMel) {    
    OATPP_LOGD("AudioWorker", "[MOCK-CPU] Receiving Audio Data Size: %ld", (long)inputAudio.size());
    OATPP_LOGD("AudioWorker", "[MOCK-CPU] Simulating CUDA processing...");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    outputMel.assign(inputAudio.size() / 2, 0.5f); // Dummy result

    OATPP_LOGI("AudioWorker", "[MOCK-CPU] Done! Result written to buffer.");
}

}}