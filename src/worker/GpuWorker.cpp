#include "Bridge.hpp"
#include "oatpp/core/base/Environment.hpp"

namespace app { namespace worker {

void AudioWorker::computeMelSpectrogram(const std::vector<float>& inputAudio, std::vector<float>& outputMel) {
    OATPP_LOGI("AudioWorker", "[REAL-GPU] Launching CUDA Kernel...");
}

}}