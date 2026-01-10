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
    
    static void assertContentType(const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request, const char* expectedType) {
        auto contentType = request->getHeader("Content-Type");
        if (!contentType || std::string(contentType->c_str()).find(expectedType) == std::string::npos) {
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
    }
};

}}

#endif /* RequestValidator_hpp */