#ifndef GlobalErrorHandler_hpp
#define GlobalErrorHandler_hpp

#include "dto/ErrorDto.hpp"
#include "exception/AppExceptions.hpp"

#include "oatpp/web/server/handler/ErrorHandler.hpp"
#include "oatpp/web/protocol/http/outgoing/ResponseFactory.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

class GlobalErrorHandler : public oatpp::web::server::handler::ErrorHandler {
private:
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_objectMapper;
public:
    GlobalErrorHandler(const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper)
        : m_objectMapper(objectMapper) {}

    std::shared_ptr<OutgoingResponse> handleError(const Status& status, const oatpp::String& message, const Headers& headers) override {
        auto errorDto = ErrorResponseDto::createShared();
        errorDto->status_code = status.code;
        errorDto->error = message;
        return ResponseFactory::createResponse(status, errorDto, m_objectMapper);
    }

    std::shared_ptr<OutgoingResponse> handleError(const std::exception_ptr& exceptionPtr) override {
        try {
            if (exceptionPtr) {
                std::rethrow_exception(exceptionPtr);
            }
        } catch (const AudioProcessingException& e) {
             auto errorDto = ErrorResponseDto::createShared();
             errorDto->status_code = 500;
             errorDto->error = "Audio Processing Error: " + oatpp::String(e.what());
             return ResponseFactory::createResponse(Status::CODE_500, errorDto, m_objectMapper);
        } catch (const std::exception& e) {
             auto errorDto = ErrorResponseDto::createShared();
             errorDto->status_code = 500;
             errorDto->error = "Internal Server Error: " + oatpp::String(e.what());
             return ResponseFactory::createResponse(Status::CODE_500, errorDto, m_objectMapper);
        } catch (...) {
             return handleError(Status::CODE_500, "Unknown Error", {});
        }
        return nullptr;
    }
};

#endif /* GlobalErrorHandler_hpp */