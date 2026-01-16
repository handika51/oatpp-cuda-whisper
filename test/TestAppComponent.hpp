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
    std::shared_ptr<AppConfig> appConfig;
    std::shared_ptr<oatpp::async::Executor> executor;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> apiObjectMapper;
    std::shared_ptr<oatpp::web::server::HttpRouter> httpRouter;
    std::shared_ptr<oatpp::web::server::handler::ErrorHandler> errorHandler;
    std::shared_ptr<oatpp::network::ConnectionHandler> serverConnectionHandler;
    std::shared_ptr<AudioWorker> audioWorker;
    std::shared_ptr<AudioService> audioService;

public:
    TestAppComponent() {
        appConfig = std::make_shared<AppConfig>();

        executor = std::make_shared<oatpp::async::Executor>(
            4, /* Data-Processing threads */
            1, /* I/O threads */
            1  /* Timer threads */
        );

        apiObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
        
        httpRouter = oatpp::web::server::HttpRouter::createShared();

        errorHandler = std::make_shared<GlobalErrorHandler>(apiObjectMapper);

        auto connectionHandler = oatpp::web::server::AsyncHttpConnectionHandler::createShared(httpRouter, executor);
        connectionHandler->setErrorHandler(errorHandler);
        serverConnectionHandler = connectionHandler;

        audioWorker = std::make_shared<AudioWorker>();

        audioService = std::make_shared<AudioService>(audioWorker);
    }
    
};

}}

#endif /* TestAppComponent_hpp */