#ifndef DTO_ProcessDto_hpp
#define DTO_ProcessDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace app { namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

class ProcessRequestDto : public oatpp::DTO {
    DTO_INIT(ProcessRequestDto, DTO)
    DTO_FIELD(String, message);
};

class ProcessResponseDto : public oatpp::DTO {
    DTO_INIT(ProcessResponseDto, DTO)
    DTO_FIELD(String, result);
};

#include OATPP_CODEGEN_END(DTO)

}}

#endif