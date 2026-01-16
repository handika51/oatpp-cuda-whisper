#include "AudioService.hpp"
#include "exception/AppExceptions.hpp"
#include <vector>

namespace app { namespace service {

using namespace app::worker;
using namespace app::exception;

AudioService::AudioService(const std::shared_ptr<AudioWorker>& audioWorker)
    : m_audioWorker(audioWorker)
{}

oatpp::String AudioService::processAudio(const oatpp::String& message) {
    std::vector<float> dummyInput(100, 0.0f); 
    std::vector<float> melOutput;
    
    m_audioWorker->computeMelSpectrogram(dummyInput, melOutput);
    
    return message;
}

oatpp::List<oatpp::Float32> AudioService::extractFeatures(const oatpp::String& rawData) {
    auto result = oatpp::List<oatpp::Float32>::createShared();
    
    // Check if data is valid (16-bit = 2 bytes)
    if (!rawData || rawData->size() % 2 != 0) {
        return result; // Return empty or throw exception
    }

    const int16_t* samples = reinterpret_cast<const int16_t*>(rawData->data());
    size_t sampleCount = rawData->size() / 2;

    // Placeholder Logic:
    // Whisper typically uses 25ms window (400 samples @ 16kHz)
    // with 10ms stride (160 samples @ 16kHz).
    // It produces 80 mel-frequency bins.
    
    size_t frameSize = 400; 
    size_t step = 160;      
    size_t n_mels = 80;

    for (size_t i = 0; i + frameSize < sampleCount; i += step) {
        // For a real implementation, you would:
        // 1. Apply Hann window
        // 2. Compute FFT
        // 3. Apply Mel filterbank
        // 4. Logarithm
        
        // HERE: We just simulate by pushing 80 dummy values (e.g. 0.0 to 1.0)
        // derived from the signal energy to allow verification of processing.
        
        float energy = 0.0f;
        for (size_t j = 0; j < frameSize; ++j) {
            float val = samples[i+j] / 32768.0f; 
            energy += val * val;
        }
        energy /= frameSize;

        // Populate 80 bins with this energy value (mock)
        for(size_t k=0; k < n_mels; ++k) {
            result->push_back(energy);
        }
    }
    
    return result;
}

}}