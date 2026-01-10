#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

#include "oatpp/core/macro/component.hpp"

#include "worker/Bridge.hpp"
#include "service/AudioService.hpp"
#include "errorhandler/GlobalErrorHandler.hpp"
#include "AppConfig.hpp"

namespace app {

using namespace app::service;
using namespace app::worker;

class AppComponent {
public: 
    
    OATPP_CREATE_COMPONENT(std::shared_ptr<AppConfig>, appConfig)([] {
        return std::make_shared<AppConfig>();
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
        return oatpp::parser::json::mapping::ObjectMapper::createShared();
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
        OATPP_COMPONENT(std::shared_ptr<AppConfig>, config);
        return oatpp::network::tcp::server::ConnectionProvider::createShared({config->host, config->port, oatpp::network::Address::IP_4});
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,httpRouter)([] {
        return oatpp::web::server::HttpRouter::createShared();
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::handler::ErrorHandler>, errorHandler)([] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);
        return std::make_shared<GlobalErrorHandler>(objectMapper);
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::handler::ErrorHandler>, errorHandler);
        auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
        connectionHandler->setErrorHandler(errorHandler);
        return connectionHandler;
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<AudioWorker>, audioWorker)([] {
        return std::make_shared<AudioWorker>();
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<AudioService>, audioService)([] {
        OATPP_COMPONENT(std::shared_ptr<AudioWorker>, worker);
        return std::make_shared<AudioService>(worker);
    }());
    
};

}

#endif /* AppComponent_hpp */