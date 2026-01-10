#ifndef TestClient_hpp
#define TestClient_hpp

#include "oatpp/web/client/ApiClient.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "dto/ProcessDto.hpp"
#include "dto/MessageDto.hpp"
#include "dto/BaseResponseDto.hpp"

namespace app { namespace test {

#include OATPP_CODEGEN_BEGIN(ApiClient)

class TestClient : public oatpp::web::client::ApiClient {
    API_CLIENT_INIT(TestClient)

    API_CALL("GET", "/hello", doHello)
    
    API_CALL("POST", "/process", doProcess, BODY_DTO(oatpp::Object<app::dto::ProcessRequestDto>, body))
};

#include OATPP_CODEGEN_END(ApiClient)

}}

#endif