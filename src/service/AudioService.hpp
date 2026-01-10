#ifndef Service_AudioService_hpp
#define Service_AudioService_hpp

#include "worker/Bridge.hpp"
#include "oatpp/core/Types.hpp"
#include <memory>

class AudioService {
private:
    std::shared_ptr<AudioWorker> m_audioWorker;
public:
    AudioService(const std::shared_ptr<AudioWorker>& audioWorker);
    
    oatpp::String processAudio(const oatpp::String& message);
};

#endif