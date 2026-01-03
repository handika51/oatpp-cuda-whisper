#ifndef MyController_hpp
#define MyController_hpp

#include "dto/DTOs.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "worker/Bridge.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class MyController : public oatpp::web::server::api::ApiController {
public:
    MyController(OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}
public:
    ENDPOINT("GET", "/hello", hello) {
        auto dto = MessageDto::createShared();
        dto->status_code = 200;
        dto->message = "Hello, World!";
        return createDtoResponse(Status::CODE_200, dto);
    }
    ENDPOINT("POST", "/process", processMessage, 
             REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        auto contentType = request->getHeader("Content-Type");
        if (!contentType || std::string(contentType->c_str()).find("application/json") == std::string::npos) {
            auto errorDto = ErrorResponseDto::createShared();
            errorDto->status_code = 415;
            errorDto->error = "Unsupported Media Type";
            return createDtoResponse(Status::CODE_415, errorDto);
        }

        auto bodyString = request->readBodyToString();
        if(!bodyString || bodyString->size() == 0) {
            auto errorDto = ErrorResponseDto::createShared();
            errorDto->status_code = 400;
            errorDto->error = "Empty Request Body";
            return createDtoResponse(Status::CODE_400, errorDto);
        }

        oatpp::Object<ProcessRequestDto> requestDto;
        try {
            auto jsonMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
            requestDto = jsonMapper->readFromString<oatpp::Object<ProcessRequestDto>>(bodyString);
        } catch (const std::exception& e) {
            auto errorDto = ErrorResponseDto::createShared();
            errorDto->status_code = 400;
            errorDto->error = "Invalid Request Body Format";
            return createDtoResponse(Status::CODE_400, errorDto);
        }
        if (!requestDto->message) {
            auto errorDto = ErrorResponseDto::createShared();
            errorDto->status_code = 400;
            errorDto->error = "Invalid Request Body";
            return createDtoResponse(Status::CODE_400, errorDto);
        }
        if (requestDto->message->size() == 0){
            auto errorDto = ErrorResponseDto::createShared();
            errorDto->status_code = 400;
            errorDto->error = "Message cannot be empty";
            return createDtoResponse(Status::CODE_400, errorDto);
        }
        
        auto responseDto = ProcessResponseDto::createShared();
        responseDto->status_code = 200;
        responseDto->message = "Success";
        responseDto->result = requestDto->message;
        return createDtoResponse(Status::CODE_200, responseDto);
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* MyController_hpp */