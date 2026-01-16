#ifndef GlobalErrorHandler_hpp
#define GlobalErrorHandler_hpp

#include "dto/ErrorDto.hpp"
#include "exception/AppExceptions.hpp"

#include "oatpp/web/server/handler/ErrorHandler.hpp"
#include "oatpp/web/protocol/http/outgoing/ResponseFactory.hpp"
#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"

namespace app {

using namespace app::dto;
using namespace app::exception;

// Typedefs untuk memudahkan akses tipe Oat++
using Status = oatpp::web::protocol::http::Status;
using OutgoingResponse = oatpp::web::protocol::http::outgoing::Response;
using ResponseFactory = oatpp::web::protocol::http::outgoing::ResponseFactory;
using Headers = oatpp::web::protocol::http::Headers;

class GlobalErrorHandler : public oatpp::web::server::handler::ErrorHandler {
private:
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_objectMapper;

    std::shared_ptr<OutgoingResponse> createJsonResponse(const Status& status, const oatpp::Void& dto) {
        auto stream = std::make_shared<oatpp::data::stream::BufferOutputStream>();
        m_objectMapper->write(stream.get(), dto);
        auto response = OutgoingResponse::createShared(status, oatpp::web::protocol::http::outgoing::BufferBody::createShared(stream->toString()));
        response->putHeader(oatpp::web::protocol::http::Header::CONTENT_TYPE, "application/json");
        return response;
    }

public:
    GlobalErrorHandler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper)
        : m_objectMapper(objectMapper) {}

    std::shared_ptr<OutgoingResponse> handleError(const Status& status, const oatpp::String& message, const Headers& headers) override {
        auto errorDto = ErrorResponseDto::createShared();
        errorDto->status_code = status.code;
        errorDto->error = message;
        return createJsonResponse(status, errorDto);
    }

    std::shared_ptr<OutgoingResponse> handleError(const std::exception_ptr& exceptionPtr) override {
        try {
            if (exceptionPtr) {
                std::rethrow_exception(exceptionPtr);
            }
        } catch (const AudioProcessingException& e) {
             auto errorDto = ErrorResponseDto::createShared();
             errorDto->status_code = 500;
             std::string msg = "Audio Processing Error: ";
             msg += e.what();
             errorDto->error = msg.c_str();
             return createJsonResponse(Status::CODE_500, errorDto);
        } catch (const ValidationException& e) {
             auto errorDto = ErrorResponseDto::createShared();
             errorDto->status_code = 400;
             std::string msg = "Validation Error: ";
             msg += e.what();
             errorDto->error = msg.c_str();
             return createJsonResponse(Status::CODE_400, errorDto);
        } catch (const std::exception& e) {
             auto errorDto = ErrorResponseDto::createShared();
             errorDto->status_code = 500;
             // Log the actual error for internal debugging, but return a generic message to the client
             // OATPP_LOGE("GlobalErrorHandler", "Internal Server Error: %s", e.what()); // Assuming a logging mechanism exists
             errorDto->error = "Internal Server Error";
             return createJsonResponse(Status::CODE_500, errorDto);
        } catch (...) {
             return handleError(Status::CODE_500, "Unknown Error", {});
        }
        return nullptr;
    }
};

}

#endif /* GlobalErrorHandler_hpp */