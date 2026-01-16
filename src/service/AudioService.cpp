#include "AudioService.hpp"
#include "exception/AppExceptions.hpp"
#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>

namespace app { namespace service {

using namespace app::worker;
using namespace app::exception;

AudioService::AudioService(const std::shared_ptr<WorkerManager>& workerManager)
    : m_workerManager(workerManager)
{}

oatpp::String AudioService::processAudio(const oatpp::String& message) {
    if(!message) return nullptr;

    ReqSlot req;
    req.type = TASK_TEXT_PROCESS;
    // Copy message
    size_t len = message->size();
    if (len >= TEXT_CHUNK_SIZE) {
        len = TEXT_CHUNK_SIZE - 1;
    }
    std::memcpy(req.text_data, message->c_str(), len);
    req.text_data[len] = '\0';
    req.len = (uint32_t)len;

    auto future = m_workerManager->submitTask(req);
    
    // Block until result
    try {
        RespSlot resp = future.get();
        if (resp.status_code != 0) {
             throw std::runtime_error("Worker returned error code " + std::to_string(resp.status_code));
        }
        return oatpp::String(resp.text_result, resp.len);
    } catch (const std::exception& e) {
        OATPP_LOGE("AudioService", "Error processing text: %s", e.what());
        throw;
    }
}

oatpp::List<oatpp::Float32> AudioService::extractFeatures(const oatpp::String& rawData) {
    auto result = oatpp::List<oatpp::Float32>::createShared();
    
    if (!rawData || rawData->size() % 2 != 0) {
        return result; 
    }

    size_t sampleCount = rawData->size() / 2;
    if (sampleCount > AUDIO_CHUNK_SIZE) {
         // Truncate for MVP
         sampleCount = AUDIO_CHUNK_SIZE;
    }

    ReqSlot req;
    req.type = TASK_AUDIO_PROCESS;
    req.audio.sample_rate = 16000;
    req.audio.num_samples = (uint32_t)sampleCount;
    
    const int16_t* pcm = reinterpret_cast<const int16_t*>(rawData->data());
    for(size_t i=0; i<sampleCount; ++i) {
        req.audio.audio_data[i] = pcm[i] / 32768.0f;
    }

    auto future = m_workerManager->submitTask(req);
    
    try {
        RespSlot resp = future.get();
        if (resp.status_code != 0) {
            throw std::runtime_error("Worker returned error code " + std::to_string(resp.status_code));
        }

        for(size_t i=0; i<resp.len; ++i) {
            result->push_back(resp.mel_features[i]);
        }
    } catch (const std::exception& e) {
         OATPP_LOGE("AudioService", "Error processing audio: %s", e.what());
         throw;
    }
    
    return result;
}

}}
