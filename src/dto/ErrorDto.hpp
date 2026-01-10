#ifndef DTO_ErrorDto_hpp
#define DTO_ErrorDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace app { namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

class ErrorResponseDto : public oatpp::DTO {
    DTO_INIT(ErrorResponseDto, DTO)
    DTO_FIELD(Int32, status_code);
    DTO_FIELD(String, error);
};

#include OATPP_CODEGEN_END(DTO)

}}

#endif