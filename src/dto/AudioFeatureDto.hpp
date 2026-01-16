#ifndef AudioFeatureDto_hpp
#define AudioFeatureDto_hpp

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

namespace app { namespace dto {

#include OATPP_CODEGEN_BEGIN(DTO)

class AudioFeatureDto : public oatpp::DTO {
  DTO_INIT(AudioFeatureDto, DTO)

  DTO_FIELD_INFO(features) {
    info->description = "Array of Mel Spectrogram features";
  }
  DTO_FIELD(List<Float32>, features);

  DTO_FIELD_INFO(sample_count) {
    info->description = "Number of audio samples processed";
  }
  DTO_FIELD(Int64, sample_count);
};

#include OATPP_CODEGEN_END(DTO)

}}

#endif /* AudioFeatureDto_hpp */
