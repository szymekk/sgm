#ifndef SGM_MEDIAN_H
#define SGM_MEDIAN_H

#include <cstdint>

#include "img/core.h"

namespace img {

img::ImageGray<std::uint8_t> median(const img::ImageGray<std::uint8_t>& image);

}

#endif //SGM_MEDIAN_H
