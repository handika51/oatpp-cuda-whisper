#ifndef DTO_BaseResponseDto_hpp
#define DTO_BaseResponseDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"
#include <cstdio>

namespace app { namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

template<class T>
class BaseResponseDto : public oatpp::DTO {
    DTO_INIT(BaseResponseDto, DTO)

    DTO_FIELD(Int32, code);
    DTO_FIELD(Boolean, is_success);
    DTO_FIELD(String, message);
    DTO_FIELD(String, duration);
    DTO_FIELD(T, result);

public:
    static oatpp::Object<BaseResponseDto<T>> createSuccess(const T& resultData, const char* msg, long long elapsedMicros) {
        auto dto = BaseResponseDto<T>::createShared();
        dto->code = 200;
        dto->is_success = true;
        dto->message = msg;
        
        double ms = elapsedMicros / 1000.0;
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.6fms", ms);
        dto->duration = buffer;
        
        dto->result = resultData;
        return dto;
    }
};

#include OATPP_CODEGEN_END(DTO)

}}

#endif