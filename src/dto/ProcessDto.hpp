#ifndef DTO_ProcessDto_hpp
#define DTO_ProcessDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ProcessRequestDto : public oatpp::DTO {
    DTO_INIT(ProcessRequestDto, DTO)
    DTO_FIELD(String, message);
};

class ProcessResponseDto : public oatpp::DTO {
    DTO_INIT(ProcessResponseDto, DTO)
    DTO_FIELD(Int32, status_code);
    DTO_FIELD(String, message);
    DTO_FIELD(String, result);
};

class ProcessResult : public oatpp::DTO {
    DTO_INIT(ProcessResult, DTO)
    DTO_FIELD(String, transcript);
};

#include OATPP_CODEGEN_END(DTO)

#endif