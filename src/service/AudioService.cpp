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

}}