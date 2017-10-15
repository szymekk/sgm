#ifndef SGM_SGM_H
#define SGM_SGM_H

#include <cstdint>
#include "img/core.h"

namespace img {

ImageGray <std::uint8_t>
semi_global_matching(const ImageGray <std::uint8_t>& left, const ImageGray <std::uint8_t>& right);

} // namespace img

#endif //SGM_SGM_H
