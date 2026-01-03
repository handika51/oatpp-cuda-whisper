#include "Bridge.hpp"
#include <iostream>
#include <thread> // Untuk simulasi delay
#include <chrono>

void computeMelSpectrogram(const std::vector<float>& inputAudio, std::vector<float>& outputMel) {    
    std::cout << "[MOCK-CPU] Receiving Audio Data Size: " << inputAudio.size() << std::endl;
    std::cout << "[MOCK-CPU] Simulating CUDA processing..." << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    outputMel.assign(inputAudio.size() / 2, 0.5f); // Dummy result

    std::cout << "[MOCK-CPU] Done! Result written to buffer." << std::endl;
}