#ifndef DTOs_hpp
#define DTOs_hpp

#include "oatpp/core/data/mapping/type/Object.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class MessageDto : public oatpp::DTO {
    DTO_INIT(MessageDto, DTO)

    DTO_FIELD(Int32, status_code);
    DTO_FIELD(String, message);
};

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

class ErrorResponseDto : public oatpp::DTO {
    DTO_INIT(ErrorResponseDto, DTO)
    DTO_FIELD(Int32, status_code);
    DTO_FIELD(String, error);
};

#include OATPP_CODEGEN_END(DTO)
#endif /* DTOs_hpp */