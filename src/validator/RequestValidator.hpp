#ifndef RequestValidator_hpp
#define RequestValidator_hpp

#include "dto/ProcessDto.hpp"
#include "exception/AppExceptions.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

namespace app { namespace validator {

using namespace app::dto;
using namespace app::exception;

class RequestValidator {
public:
    static const v_int32 MAX_MESSAGE_LENGTH = 1024; // Define a reasonable max length

    static void assertContentType(const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request, const char* expectedType) {
        auto contentType = request->getHeader("Content-Type");
        if (!contentType || *contentType != expectedType) { // Stricter check
            throw ValidationException("Unsupported Media Type");
        }
    }

    template<typename T>
    static oatpp::Object<T> parseBody(const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request, 
                                      const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper) {
        try {
            return request->readBodyToDto<oatpp::Object<T>>(objectMapper);
        } catch (const oatpp::parser::ParsingError& e) {
            throw ValidationException("Invalid Request Body Format");
        } catch (const std::exception& e) {
            throw ValidationException("Internal Server Error during parsing");
        }
    }

    static void validateProcessRequest(const oatpp::Object<ProcessRequestDto>& dto) {
        if (!dto || !dto->message || dto->message->size() == 0) {
            throw ValidationException("Message cannot be empty");
        }
        if (dto->message->size() > MAX_MESSAGE_LENGTH) { // Enforce max length
            throw ValidationException("Message too long");
        }
    }
};

}}

#endif /* RequestValidator_hpp */