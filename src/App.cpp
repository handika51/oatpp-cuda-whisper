#include "AppComponent.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "controller/MyController.hpp"

using namespace app;
using namespace app::controller;

void run() {
    AppComponent components;

    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    auto myController = std::make_shared<MyController>();
    router->addController(myController);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    oatpp::network::Server server(connectionProvider, connectionHandler);

    OATPP_LOGI("App", "Server running on port %s", connectionProvider->getProperty("port").toString()->c_str());
    
    server.run();
}

int main(int argc, const char * argv[]) {
    oatpp::base::Environment::init();

    run();

    oatpp::base::Environment::destroy();
    return 0;
}

