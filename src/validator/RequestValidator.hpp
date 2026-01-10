#ifndef RequestValidator_hpp
#define RequestValidator_hpp

#include "dto/ProcessDto.hpp"
#include "oatpp/web/protocol/http/HttpError.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

class RequestValidator {
public:
    
    static void assertContentType(const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request, const char* expectedType) {
        auto contentType = request->getHeader("Content-Type");
        if (!contentType || std::string(contentType->c_str()).find(expectedType) == std::string::npos) {
            throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_415, "Unsupported Media Type");
        }
    }

    template<typename T>
    static oatpp::Object<T> parseBody(const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request, 
                                      const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper) {
        try {
            return request->readBodyToDto<oatpp::Object<T>>(objectMapper);
        } catch (const oatpp::parser::ParsingError& e) {
            throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_400, "Invalid Request Body Format");
        } catch (const std::exception& e) {
            throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_500, "Internal Server Error");
        }
    }

    static void validateProcessRequest(const oatpp::Object<ProcessRequestDto>& dto) {
        if (!dto || !dto->message || dto->message->size() == 0) {
            throw oatpp::web::protocol::http::HttpError(oatpp::web::protocol::http::Status::CODE_400, "Message cannot be empty");
        }
    }
};

#endif /* RequestValidator_hpp */