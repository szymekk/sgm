#ifndef SGM_SGM_H
#define SGM_SGM_H

#include <array>
#include <cstdint>
#include "img/core.h"

namespace img {

inline const std::size_t MAX_DISPARITY = 64;

using cost_t = std::uint8_t;
using cost_arr_t = std::array<cost_t, MAX_DISPARITY>;
inline const cost_t INVALID_COST = std::numeric_limits<cost_t>::max();

using acc_cost_t = std::uint16_t;
using acc_cost_arr_t = std::array<acc_cost_t, MAX_DISPARITY>;

using cost_function_t = cost_t(
        const img::ImageGray<std::uint8_t>& left,
        const img::ImageGray<std::uint8_t>& right,
        const std::size_t row,
        const std::size_t col,
        const std::size_t disparity);

cost_t pixelwise_absolute_difference(
        const img::ImageGray<std::uint8_t>& left,
        const img::ImageGray<std::uint8_t>& right,
        const std::size_t row,
        const std::size_t col,
        const std::size_t disparity);

template<std::size_t WINDOW_SIZE>
cost_t sum_of_absolute_differences(
        const img::ImageGray<std::uint8_t>& left,
        const img::ImageGray<std::uint8_t>& right,
        const std::size_t row,
        const std::size_t col,
        const std::size_t disparity);

img::Grid<cost_arr_t>
calculate_costs(const img::ImageGray<std::uint8_t>& left, const img::ImageGray<std::uint8_t>& right,
                cost_function_t cost_function);

img::Grid<acc_cost_arr_t>
accumulate_costs_direction_x1_y0(
        const img::ImageGray<std::uint8_t>& left,
        const img::Grid<cost_arr_t>& costs);

ImageGray <std::uint8_t>
semi_global_matching(const ImageGray <std::uint8_t>& left, const ImageGray <std::uint8_t>& right);

} // namespace img

#endif //SGM_SGM_H
