#include "AudioServiceTest.hpp"
#include "service/AudioService.hpp"
#include "worker/WorkerManager.hpp"
#include "worker/WorkerMain.hpp"
#include <iostream>
#include <thread>
#include <vector>

namespace app { namespace test {

void AudioServiceTest::onRun() {
    // 1. Setup Host
    auto manager = std::make_shared<app::worker::WorkerManager>();
    // Start host, 0 subprocesses because we will run worker in a thread
    manager->start(0, nullptr); 

    // 2. Setup Worker (Thread)
    std::thread workerThread([]{
        // Small delay to let host init
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app::worker::runWorker();
    });

    // 3. Service
    app::service::AudioService service(manager);

    try {
        {
            // Test: processAudio (Text)
            oatpp::String input = "test message";
            auto result = service.processAudio(input);
            // WorkerMain reverses the string
            OATPP_LOGD("Test", "Result: %s", result->c_str());
            OATPP_ASSERT(result == "egassem tset"); 
        }

        {
            // Test: extractFeatures
            // Need > 400 samples. 401 samples = 802 bytes.
            std::vector<int16_t> samples(401, 1000); 
            oatpp::String data(reinterpret_cast<const char*>(samples.data()), samples.size() * 2);
            
            auto features = service.extractFeatures(data);
            
            // CpuMock returns input.size()/2 items. 
            // 401 / 2 = 200 items.
            OATPP_ASSERT(features->size() == 200);
            OATPP_ASSERT(features->front() == 0.5f);
        }
    } catch (const std::exception& e) {
        OATPP_LOGE("Test", "Exception: %s", e.what());
        // Signal shutdown to ensure thread joins
        manager->sendShutdownSignal();
        if(workerThread.joinable()) workerThread.join();
        manager->stop();
        throw;
    }
    
    // Cleanup
    manager->sendShutdownSignal();
    if(workerThread.joinable()) {
        workerThread.join();
    }
    manager->stop();
}

}}
