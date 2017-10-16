#ifndef SGM_SGM_H
#define SGM_SGM_H

#include <array>
#include <cstdint>
#include "img/core.h"

namespace img {

namespace detail {
template<typename ContainerT>
std::size_t index_of_min_element(const ContainerT& c) {
    const auto min_element = std::min_element(std::begin(c), std::end(c));
    return static_cast<std::size_t>(std::distance(std::begin(c), min_element));
}

} // namespace detail

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

cost_function_t pixelwise_absolute_difference;

template<std::size_t WINDOW_SIZE>
cost_function_t sum_of_absolute_differences;

img::Grid<cost_arr_t>
calculate_costs(const img::ImageGray<std::uint8_t>& left, const img::ImageGray<std::uint8_t>& right,
                cost_function_t cost_function);

img::Grid<acc_cost_arr_t>
accumulate_costs_direction_x1_y0(
        const img::ImageGray<std::uint8_t>& left,
        const img::Grid<cost_arr_t>& costs);

template<typename CostArrayT>
ImageGray<std::uint8_t> create_disparity_view(const img::Grid<CostArrayT>& costs) {
    static_assert(std::numeric_limits<std::uint8_t>::max() >= MAX_DISPARITY,
                  "disparity values might not fit into the image");

    ImageGray<std::uint8_t> disparity_view(costs.width, costs.height);
    std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(disparity_view.data),
                   [](const auto& cv) {
                       const auto disparity = detail::index_of_min_element(cv);
                       static_assert(MAX_DISPARITY == 64, "can't scale by shifting two bits");
                       const auto scaled_disparity = static_cast<std::uint8_t>(4 * disparity);
                       return PixelGray<std::uint8_t>{scaled_disparity};
                   });
    return disparity_view;
}

ImageGray <std::uint8_t>
semi_global_matching(const ImageGray <std::uint8_t>& left, const ImageGray <std::uint8_t>& right);

} // namespace img

#endif //SGM_SGM_H
