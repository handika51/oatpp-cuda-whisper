#include "AppComponent.hpp"
#include "worker/WorkerMain.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "controller/MyController.hpp"
#include <iostream>
#include <cstring>

using namespace app;
using namespace app::controller;

void run(const char* execPath) {
    AppComponent components;

    // Start Worker Manager
    OATPP_COMPONENT(std::shared_ptr<app::worker::WorkerManager>, workerManager);
    
    // Start 4 workers by default (as per requirements target)
    // We pass the executable path so manager can fork/exec
    workerManager->start(4, execPath);

    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);
    OATPP_COMPONENT(std::shared_ptr<app::service::AudioService>, audioService);

    auto myController = std::make_shared<MyController>(objectMapper, audioService);
    router->addController(myController);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    oatpp::network::Server server(connectionProvider, connectionHandler);

    OATPP_LOGI("App", "Server running on port %s", connectionProvider->getProperty("port").toString()->c_str());
    
    server.run();

    // Stop workers on exit
    workerManager->stop();
}

int main(int argc, const char * argv[]) {
    // Check for worker flag
    if (argc > 1 && strcmp(argv[1], "--worker") == 0) {
        // Run as Worker Process
        app::worker::runWorker();
        return 0;
    }

    oatpp::base::Environment::init();

    // Pass argv[0] to run() for re-launching workers
    run(argv[0]);

    oatpp::base::Environment::destroy();
    return 0;
}