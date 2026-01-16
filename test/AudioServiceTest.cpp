#include "AudioServiceTest.hpp"
#include "service/AudioService.hpp"
#include "worker/Bridge.hpp"
#include <iostream>

namespace app { namespace test {

// Simple mock for AudioWorker to verify calls if needed (though current CpuMock does logging)
// For unit testing Service logic, we rely on the injected Worker.
// Since AudioWorker is not pure virtual, we use the real instance which is a mock in test env.

void AudioServiceTest::onRun() {
    auto worker = std::make_shared<app::worker::AudioWorker>();
    app::service::AudioService service(worker);

    {
        // Test: processAudio
        oatpp::String input = "test message";
        auto result = service.processAudio(input);
        OATPP_ASSERT(result == "test message"); // Echo behavior
    }

    {
        // Test: extractFeatures with valid data
        // 10 bytes = 5 int16 samples
        // 'a' = 0x61, 'b' = 0x62
        std::string raw(10, 'a'); 
        oatpp::String data = oatpp::String(raw.c_str(), raw.size());
        
        auto features = service.extractFeatures(data);
        OATPP_ASSERT(features);
        // Logic check: sampleCount = 10/2 = 5. 
        // Mock logic: 
        // frameSize=400, step=160.
        // loop i=0; i+400 < 5 ... loop condition fails immediately for small input.
        // So features list should be empty or contain nothing if logic strictly follows loop.
        
        // However, let's verify it doesn't crash.
        OATPP_ASSERT(features->size() == 0); 
    }

    {
        // Test: extractFeatures with odd-byte data (invalid for 16-bit)
        std::string raw(3, 'a');
        oatpp::String data = oatpp::String(raw.c_str(), raw.size());
        
        // Expecting safe handling (empty list returned)
        auto features = service.extractFeatures(data);
        OATPP_ASSERT(features);
        OATPP_ASSERT(features->size() == 0);
    }
    
    {
        // Test: extractFeatures with enough data to trigger loop
        // Need > 400 samples. 401 samples = 802 bytes.
        std::vector<int16_t> samples(401, 1000); // 401 samples with value 1000
        oatpp::String data(reinterpret_cast<const char*>(samples.data()), samples.size() * 2);
        
        auto features = service.extractFeatures(data);
        // Loop runs once for i=0 (0+400 < 401).
        // Should push 80 mel bins.
        OATPP_ASSERT(features->size() == 80);
    }
}

}}