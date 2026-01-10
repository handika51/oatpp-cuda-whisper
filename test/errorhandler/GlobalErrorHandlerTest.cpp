#include "GlobalErrorHandlerTest.hpp"
#include "errorhandler/GlobalErrorHandler.hpp"
#include "exception/AppExceptions.hpp"

#include "oatpp/core/base/Environment.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp" // Correct include for BufferOutputStream

namespace app { namespace test { namespace errorhandler {

GlobalErrorHandlerTest::GlobalErrorHandlerTest() : UnitTest("TEST[GlobalErrorHandlerTest]") {}

void GlobalErrorHandlerTest::onRun() {
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    app::GlobalErrorHandler errorHandler(objectMapper);

    OATPP_LOGI(TAG, "Testing AudioProcessingException...");
    try {
        throw app::exception::AudioProcessingException("Audio processing failed");
    } catch (...) {
        auto exceptionPtr = std::current_exception();
        auto response = errorHandler.handleError(exceptionPtr);
        
        OATPP_ASSERT(response);
        OATPP_ASSERT(response->getStatus().code == 500);

        auto stream = std::make_shared<oatpp::data::stream::BufferOutputStream>();
        response->send(stream.get(), nullptr, nullptr); // Use .get() to pass raw pointer
        auto body = stream->toString();
        
        OATPP_ASSERT(body->find("Audio Processing Error: Audio processing failed") != std::string::npos);
    }

    OATPP_LOGI(TAG, "Testing ValidationException...");
    try {
        throw app::exception::ValidationException("Invalid input");
    } catch (...) {
        auto exceptionPtr = std::current_exception();
        auto response = errorHandler.handleError(exceptionPtr);
        
        OATPP_ASSERT(response);
        OATPP_ASSERT(response->getStatus().code == 400);

        auto stream = std::make_shared<oatpp::data::stream::BufferOutputStream>();
        response->send(stream.get(), nullptr, nullptr);
        auto body = stream->toString();
        
        OATPP_ASSERT(body->find("Validation Error: Invalid input") != std::string::npos);
    }

    OATPP_LOGI(TAG, "Testing Generic std::exception (Security check)...");
    try {
        throw std::runtime_error("Sensitive internal info");
    } catch (...) {
        auto exceptionPtr = std::current_exception();
        auto response = errorHandler.handleError(exceptionPtr);
        
        OATPP_ASSERT(response);
        OATPP_ASSERT(response->getStatus().code == 500);

        auto stream = std::make_shared<oatpp::data::stream::BufferOutputStream>();
        response->send(stream.get(), nullptr, nullptr);
        auto body = stream->toString();

        OATPP_ASSERT(body->find("Internal Server Error") != std::string::npos);
        OATPP_ASSERT(body->find("Sensitive internal info") == std::string::npos);
    }

    OATPP_LOGI(TAG, "Testing Unknown Exception...");
    try {
        throw 42; // Throwing int
    } catch (...) {
        auto exceptionPtr = std::current_exception();
        auto response = errorHandler.handleError(exceptionPtr);
        
        OATPP_ASSERT(response);
        OATPP_ASSERT(response->getStatus().code == 500);
        
        auto stream = std::make_shared<oatpp::data::stream::BufferOutputStream>();
        response->send(stream.get(), nullptr, nullptr);
        auto body = stream->toString();

        OATPP_ASSERT(body->find("Unknown Error") != std::string::npos);
    }

    OATPP_LOGI(TAG, "Testing handleError with Status...");
    {
        auto response = errorHandler.handleError(oatpp::web::protocol::http::Status::CODE_404, "Resource Not Found", {});
        OATPP_ASSERT(response);
        OATPP_ASSERT(response->getStatus().code == 404);

        auto stream = std::make_shared<oatpp::data::stream::BufferOutputStream>();
        response->send(stream.get(), nullptr, nullptr);
        auto body = stream->toString();

        OATPP_ASSERT(body->find("Resource Not Found") != std::string::npos);
    }
}

}}}
