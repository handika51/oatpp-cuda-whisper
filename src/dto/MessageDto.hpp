#ifndef DTO_MessageDto_hpp
#define DTO_MessageDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace app { namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

class MessageDto : public oatpp::DTO {
    DTO_INIT(MessageDto, DTO)

    DTO_FIELD(Int32, status_code);
    DTO_FIELD(String, message);
};

#include OATPP_CODEGEN_END(DTO)

}}

#endif