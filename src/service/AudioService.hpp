#ifndef Service_AudioService_hpp
#define Service_AudioService_hpp

#include "worker/WorkerManager.hpp"
#include "oatpp/core/Types.hpp"
#include <memory>

namespace app { namespace service {

using namespace app::worker;

class AudioService {
private:
    std::shared_ptr<WorkerManager> m_workerManager;
public:
    AudioService(const std::shared_ptr<WorkerManager>& workerManager);
    
    oatpp::String processAudio(const oatpp::String& message);

    /**
     * Sends raw PCM 16-bit mono 16kHz audio to Worker Process for Mel Spectrogram computation.
     */
    oatpp::List<oatpp::Float32> extractFeatures(const oatpp::String& rawData);
};

}}

#endif