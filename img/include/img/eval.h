#ifndef SGM_EVAL_H
#define SGM_EVAL_H

#include <cstdint>

#include "img/core.h"

namespace img {

using quality_metric_t = double;

quality_metric_t
root_mean_square_error(const img::ImageGray<std::uint8_t>& img_first,
                       const img::ImageGray<std::uint8_t>& img_second);

quality_metric_t
percentage_of_bad_pixels(const img::ImageGray<std::uint8_t>& img_first,
                         const img::ImageGray<std::uint8_t>& img_second,
                         std::uint8_t threshold);

} // namespace img

#endif //SGM_EVAL_H
