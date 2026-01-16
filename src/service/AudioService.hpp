#ifndef Service_AudioService_hpp
#define Service_AudioService_hpp

#include "worker/Bridge.hpp"
#include "oatpp/core/Types.hpp"
#include <memory>

namespace app { namespace service {

using namespace app::worker;

class AudioService {
private:
    std::shared_ptr<AudioWorker> m_audioWorker;
public:
    AudioService(const std::shared_ptr<AudioWorker>& audioWorker);
    
    oatpp::String processAudio(const oatpp::String& message);

    /**
     * Placeholder to convert raw PCM 16-bit mono 16kHz audio to Mel Spectrogram features.
     * Returns a list of features (flattened or structured).
     */
    oatpp::List<oatpp::Float32> extractFeatures(const oatpp::String& rawData);
};

}}

#endif