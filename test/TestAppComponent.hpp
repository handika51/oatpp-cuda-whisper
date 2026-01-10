#ifndef TestAppComponent_hpp
#define TestAppComponent_hpp

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/async/Executor.hpp"

#include "worker/Bridge.hpp"
#include "service/AudioService.hpp"
#include "errorhandler/GlobalErrorHandler.hpp"
#include "AppConfig.hpp"

namespace app { namespace test {

using namespace app::service;
using namespace app::worker;

class TestAppComponent {
public: 
    
    OATPP_CREATE_COMPONENT(std::shared_ptr<AppConfig>, appConfig)([] {
        return std::make_shared<AppConfig>();
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([] {
        return std::make_shared<oatpp::async::Executor>(
            4, /* Data-Processing threads */
            1, /* I/O threads */
            1  /* Timer threads */
        );
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
        return oatpp::parser::json::mapping::ObjectMapper::createShared();
    }());

    // OMITTED: serverConnectionProvider to avoid binding to port 8000 during tests

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,httpRouter)([] {
        return oatpp::web::server::HttpRouter::createShared();
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::handler::ErrorHandler>, errorHandler)([] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);
        return std::make_shared<GlobalErrorHandler>(objectMapper);
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::handler::ErrorHandler>, errorHandler);
        auto connectionHandler = oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, executor);
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

}}

#endif /* TestAppComponent_hpp */