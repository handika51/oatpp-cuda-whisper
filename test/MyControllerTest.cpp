#include "MyControllerTest.hpp"
#include "app/TestClient.hpp"
#include "TestAppComponent.hpp"
#include "controller/MyController.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/virtual_/client/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/Interface.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/network/Server.hpp"
#include <thread>
#include <iostream>

namespace app { namespace test {

struct ServerThreadGuard {
    oatpp::network::Server* server;
    std::shared_ptr<oatpp::network::virtual_::server::ConnectionProvider> serverConnectionProvider;
    std::shared_ptr<oatpp::network::virtual_::client::ConnectionProvider> clientConnectionProvider;
    std::thread thread;

    ServerThreadGuard(oatpp::network::Server* s, 
                      const std::shared_ptr<oatpp::network::virtual_::server::ConnectionProvider>& scp,
                      const std::shared_ptr<oatpp::network::virtual_::client::ConnectionProvider>& ccp) 
        : server(s), serverConnectionProvider(scp), clientConnectionProvider(ccp)
    {
        thread = std::thread([this]{
            try {
                server->run();
            } catch (const std::exception& e) {
                OATPP_LOGE("ServerThread", "Error: %s", e.what());
            } catch (...) {
                OATPP_LOGE("ServerThread", "Unknown Error");
            }
        });
    }

    ~ServerThreadGuard() {
        OATPP_LOGD("ServerThreadGuard", "Destructor called. Stopping components...");
        if(server) {
            OATPP_LOGD("ServerThreadGuard", "Stopping server...");
            server->stop();
        }
        if(serverConnectionProvider) {
            OATPP_LOGD("ServerThreadGuard", "Stopping server provider...");
            serverConnectionProvider->stop();
        }
        if(clientConnectionProvider) {
            OATPP_LOGD("ServerThreadGuard", "Stopping client provider...");
            clientConnectionProvider->stop();
        }
        if(thread.joinable()) {
            OATPP_LOGD("ServerThreadGuard", "Joining thread...");
            thread.join();
            OATPP_LOGD("ServerThreadGuard", "Thread joined.");
        }
    }
};

void MyControllerTest::onRun() {
    
    { // Scope block to force destruction of components before function exit
        
        // 1. Initialize Components
        TestAppComponent component; 
        
        // 2. Create Virtual Interface
        auto interface = oatpp::network::virtual_::Interface::obtainShared("virtual-test-interface");

        // 3. Override Connection Provider for Virtual Interface
        auto connectionProvider = oatpp::network::virtual_::server::ConnectionProvider::createShared(interface);
        
        // Create MyController with manual dependency injection
        auto myController = std::make_shared<app::controller::MyController>(component.apiObjectMapper, component.audioService);
        component.httpRouter->addController(myController);

        // 4. Create Server
        oatpp::network::Server server(connectionProvider, component.serverConnectionHandler);
        
        // 5. Create Client
        auto clientConnectionProvider = oatpp::network::virtual_::client::ConnectionProvider::createShared(interface);
        
        // RAII Guard to ensure server stops and thread joins
        ServerThreadGuard serverGuard(&server, connectionProvider, clientConnectionProvider);
        
        auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(clientConnectionProvider);
        auto client = TestClient::createShared(requestExecutor, component.apiObjectMapper);
        
        // 6. Execute Tests
        
        // /hello
        auto response = client->doHello();
        OATPP_ASSERT(response->getStatusCode() == 200);
        auto message = response->template readBodyToDto<oatpp::Object<app::dto::BaseResponseDto<oatpp::Object<app::dto::MessageDto>>>>(component.apiObjectMapper);
        OATPP_ASSERT(message);
        OATPP_ASSERT(message->code == 200);
        OATPP_ASSERT(message->result->message == "Hello, World!");

        // /process
        auto reqDto = app::dto::ProcessRequestDto::createShared();
        reqDto->message = "Test Audio Data";
        auto responseProcess = client->doProcess(reqDto);
        OATPP_ASSERT(responseProcess->getStatusCode() == 200);
        auto processResponse = responseProcess->template readBodyToDto<oatpp::Object<app::dto::ProcessResponseDto>>(component.apiObjectMapper);
        OATPP_ASSERT(processResponse);
        OATPP_ASSERT(processResponse->result == "Test Audio Data"); // Mock echoes the message

        // /audio/stream
        // Mock logic requires > 400 samples (800 bytes). Let's send 1000 bytes.
        std::string rawAudio(1000, 'a'); 
        oatpp::String audioData(rawAudio.c_str(), rawAudio.size());
        
        auto responseAudio = client->streamAudio(audioData);
        OATPP_ASSERT(responseAudio->getStatusCode() == 200);
        auto featureDto = responseAudio->template readBodyToDto<oatpp::Object<app::dto::AudioFeatureDto>>(component.apiObjectMapper);
        OATPP_ASSERT(featureDto);
        OATPP_ASSERT(featureDto->sample_count == 500);
        OATPP_ASSERT(featureDto->features->size() > 0);

        // /audio/stream - Empty Body
        auto responseAudioEmpty = client->streamAudio("");
        OATPP_ASSERT(responseAudioEmpty->getStatusCode() == 200);
        auto featureDtoEmpty = responseAudioEmpty->template readBodyToDto<oatpp::Object<app::dto::AudioFeatureDto>>(component.apiObjectMapper);
        OATPP_ASSERT(featureDtoEmpty);
        OATPP_ASSERT(featureDtoEmpty->sample_count == 0);
        OATPP_ASSERT(featureDtoEmpty->features->size() == 0);

        // 7. Cleanup
        
        // Allow threads to finish a bit gracefully (optional)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        OATPP_LOGD("MyControllerTest", "Tests finished. Explicitly stopping executor...");
        component.executor->stop();
        component.executor->join();
        
    } // End of inner scope - everything destroyed here

    OATPP_LOGD("MyControllerTest", "Executor stopped. Exiting onRun...");
}

}}