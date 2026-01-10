#ifndef MyController_hpp
#define MyController_hpp

#include "dto/MessageDto.hpp"
#include "dto/ProcessDto.hpp"
#include "dto/ErrorDto.hpp"
#include "dto/BaseResponseDto.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "service/AudioService.hpp"
#include "validator/RequestValidator.hpp"
#include "utils/ExecutionTimer.hpp"

namespace app { namespace controller {

using namespace app::dto;
using namespace app::service;
using namespace app::validator;
using namespace app::utils;

#include OATPP_CODEGEN_BEGIN(ApiController)

class MyController : public oatpp::web::server::api::ApiController {
private:
    std::shared_ptr<AudioService> m_audioService;

public:
    MyController(OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper),
                 OATPP_COMPONENT(std::shared_ptr<AudioService>, audioService))
        : oatpp::web::server::api::ApiController(objectMapper)
        , m_audioService(audioService) 
    {}
public:
    ENDPOINT("GET", "/hello", hello) {
        ExecutionTimer timer;
        
        auto result = MessageDto::createShared();
        result->status_code = 200;
        result->message = "Hello, World!";
        
        auto response = BaseResponseDto<oatpp::Object<MessageDto>>::createSuccess(result, "success", timer.getElapsedMicros());
        return createDtoResponse(Status::CODE_200, response);
    }
    ENDPOINT("POST", "/process", processMessage, 
             REQUEST(std::shared_ptr<IncomingRequest>, request)) {
        ExecutionTimer timer;
        
        RequestValidator::assertContentType(request, "application/json");

        auto requestDto = RequestValidator::parseBody<ProcessRequestDto>(request, getDefaultObjectMapper());

        RequestValidator::validateProcessRequest(requestDto);
        
        auto resultMessage = m_audioService->processAudio(requestDto->message);

        auto resultPayload = ProcessResult::createShared();
        resultPayload->transcript = resultMessage;

        auto response = BaseResponseDto<oatpp::Object<ProcessResult>>::createSuccess(resultPayload, "success", timer.getElapsedMicros());
        return createDtoResponse(Status::CODE_200, response);
    }
};

#include OATPP_CODEGEN_END(ApiController)

}}

#endif /* MyController_hpp */