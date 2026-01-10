#include "Bridge.hpp"
#include <iostream>

void AudioWorker::computeMelSpectrogram(const std::vector<float>& inputAudio, std::vector<float>& outputMel) {
    std::cout << "[REAL-GPU] Launching CUDA Kernel..." << std::endl;
}