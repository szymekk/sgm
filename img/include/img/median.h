#ifndef SGM_MEDIAN_H
#define SGM_MEDIAN_H

#include <cstdint>

#include "img/core.h"

namespace img {

enum class BoundaryTreatment {
    set_to_zero,
    copy_from_input,
    zero_pad_input
};

img::ImageGray<std::uint8_t>
median(const img::ImageGray<std::uint8_t>& image, BoundaryTreatment boundary_treatment);

}

#endif //SGM_MEDIAN_H
