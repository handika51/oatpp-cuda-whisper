#ifndef WORKER_BRIDGE_HPP
#define WORKER_BRIDGE_HPP

#include <vector>
#include <cstdint>

void computeMelSpectrogram(const std::vector<float>& inputAudio, std::vector<float>& outputMel);

#endif