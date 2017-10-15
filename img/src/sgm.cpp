#include <array>
#include <algorithm>
#include <iostream>

#include "img/fileio.h"
#include "img/sgm.h"

namespace {
const std::size_t MAX_DISPARITY = 64;

using cost_value_t = std::uint8_t;
using costs_vector_t = std::array<cost_value_t, MAX_DISPARITY>;
const cost_value_t INVALID_COST = std::numeric_limits<cost_value_t>::max();

using accumulated_cost_value_t = std::uint16_t;
using accumulated_costs_vector_t = std::array<accumulated_cost_value_t, MAX_DISPARITY>;

const accumulated_cost_value_t PENALTY_1 = 15;
const accumulated_cost_value_t PENALTY_2 = 100;

// std::uint8_t abs_diff(const std::uint8_t a, const std::uint8_t b) {
//     return static_cast<std::uint8_t>(a < b ? b - a : a - b);
// }

template<typename T>
T abs_diff(const T a, const T b) {
    return static_cast<T>(a < b ? b - a : a - b);
}

using cost_function_t = cost_value_t (*)(
        const img::ImageGray<std::uint8_t>& left,
        const img::ImageGray<std::uint8_t>& right,
        const std::size_t row,
        const std::size_t col,
        const std::size_t disparity);

// todo: add different cost functions
img::Grid<costs_vector_t>
calculate_costs(const img::ImageGray<std::uint8_t>& left, const img::ImageGray<std::uint8_t>& right,
                cost_function_t cost_function) {

    img::Grid<costs_vector_t> costs(left.width, left.height);
    for(std::size_t row = 0; row < left.height; ++row) {
        std::cerr << "processing row: " << row << "\n";
        for(std::size_t col = 0; col < left.width; ++col) {
            // std::cerr << "processing column: " << col << "\n";
            for(std::size_t d = 0; d < MAX_DISPARITY; ++d) {
                // std::cerr << "processing disparity: " << d << "\n";
                if (col < d) {
                    costs.get(col, row)[d] = INVALID_COST;
                } else {
                    // std::cerr << "c: " << col << ", r: " << row << ", d: " << d << "...";
                    const cost_value_t calculated = cost_function(left, right, row, col, d);
                    // std::cerr << " calculated: " << +calculated << "...";
                    costs.get(col, row)[d] = calculated;
                    // std::cerr << "assigned!\n";
                }
            }
        }
    }
    return costs;
}
[[maybe_unused]]
cost_value_t pixelwise_absolute_difference(
        const img::ImageGray<std::uint8_t>& left,
        const img::ImageGray<std::uint8_t>& right,
        const std::size_t row,
        const std::size_t col,
        const std::size_t disparity) {
    if (col < disparity) {
        return INVALID_COST;
    } else {
        return abs_diff(left.get(col, row).value, right.get(col-disparity, row).value);
    }
}

cost_value_t sum_of_absolute_differences(
        const img::ImageGray<std::uint8_t>& left,
        const img::ImageGray<std::uint8_t>& right,
        const std::size_t row,
        const std::size_t col,
        const std::size_t disparity,
        const std::size_t window_size) {
    assert(window_size > 0);

    const std::size_t n = (window_size - 1) / 2;
    unsigned int sum = 0;
    std::size_t valid_pixels = 0;

    assert(
            std::numeric_limits<decltype(sum)>::max() >=
            window_size * window_size * std::numeric_limits<cost_value_t>::max());


    const std::size_t r_min = (row >= n) ? row - n : 0;
    const std::size_t r_max = std::clamp<std::size_t>(row + n, 0, left.height - 1);

    const std::size_t c_min = (col >= n) ? col - n : 0;
    const std::size_t c_max = std::clamp<std::size_t>(col + n, 0, left.width - 1);

    // std::cerr << "r(" << r_min << "-" << r_max << "), c(" << c_min << "-" << c_max << ")\n";

    for (std::size_t r = r_min; r <= r_max; ++r) {
        for (std::size_t c = c_min; c <= c_max; ++c) {
            if (r < left.height && c < left.width && c >= disparity) { // r, c always >= 0 (unsigned)
                ++valid_pixels;
                sum += abs_diff(left.get(c, r).value, right.get(c - disparity, r).value);
            } else {
                // std::cerr << "invalid pixel: "
                // << "row: " << row
                // << ", col: " << col
                // << ", disparity: " << disparity
                // << "\n";
            }
        }
    }
    if (0 == valid_pixels) {
        std::cerr
            << "row: " << row
            << ", col: " << col
            << ", disparity: " << disparity
            << "\n";
    }
    assert(valid_pixels > 0);
    // std::cerr << "sum: " << sum << ", valid_pixels: " << valid_pixels << ", result: " << sum / valid_pixels << "\n";
    return static_cast<cost_value_t>(sum / valid_pixels);
}

cost_value_t sum_of_absolute_differences_3(
        const img::ImageGray<std::uint8_t>& left,
        const img::ImageGray<std::uint8_t>& right,
        const std::size_t row,
        const std::size_t col,
        const std::size_t disparity) {
    return sum_of_absolute_differences(left, right, row, col, disparity, 3);
}

accumulated_costs_vector_t
compute_additional_cost(
        const accumulated_costs_vector_t& previous,
        const std::uint8_t intensity_change) {

    using penalty_t = accumulated_cost_value_t;
    const penalty_t penalty_2_tentative =
        static_cast<penalty_t>(intensity_change ? PENALTY_2 / intensity_change : PENALTY_2);
    const penalty_t penalty_2 = std::max(PENALTY_1, penalty_2_tentative);

    accumulated_costs_vector_t additional_costs;
    for (std::size_t d = 0; d < MAX_DISPARITY; ++d) {
        auto additional_cost = std::numeric_limits<accumulated_cost_value_t>::max();
        for (std::size_t d_p = 0; d_p < MAX_DISPARITY; ++d_p) {
            const auto diff = abs_diff(d_p, d);
            if (0 == diff) {
                additional_cost = std::min(additional_cost, previous[d_p]);
            } else if (1 == diff) {
                additional_cost = std::min(additional_cost, static_cast<penalty_t>(previous[d_p]+PENALTY_1));
            } else {
                additional_cost = std::min(additional_cost, static_cast<penalty_t>(previous[d_p]+penalty_2));
            }
        }
        additional_costs[d] = additional_cost;
    }
    return additional_costs;
}

// accumulated cost at location (c, r) is equal to the sum of three terms:
// 1) initial cost at location (c, r)
// 2) additional cost depending on:
//    a) accumulated cost at previous location (c - 1, r),
//    b) the absolute difference of intensity values at the current and previous locations
// 3) negated value of the minimal element of term 2a
img::Grid<accumulated_costs_vector_t>
accumulate_costs_direction_x1_y0(
        const img::ImageGray<std::uint8_t>& left,
        const img::Grid<costs_vector_t>& costs) {

    img::Grid<accumulated_costs_vector_t> accumulated_costs(costs.width, costs.height);
    // (1) initialize accumulated costs with values from costs
    std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(accumulated_costs.data),
            [](const costs_vector_t& cv){
                accumulated_costs_vector_t acc_cv;
                std::copy(cv.begin(), cv.end(), std::begin(acc_cv));
                return acc_cv;
            });

    for (std::size_t r = 0; r < accumulated_costs.height; ++r) {
        for (std::size_t c = 1; c < accumulated_costs.width; ++c) {
            // (2)
            const auto intensity_change = abs_diff(left.get(c, r).value, left.get(c - 1, r).value);
            const auto& previous = accumulated_costs.get(c - 1,r);
            const auto additional_costs =
                    compute_additional_cost(previous, intensity_change);
            // accumulated_costs.get(c, r) += additional_costs;
            auto& local = accumulated_costs.get(c, r);
            std::transform(local.begin(), local.end(), additional_costs.begin(),
                    local.begin(), std::plus<accumulated_cost_value_t>());

            // (3) substract min_element of previous from each element of local
            const auto min = *std::min_element(previous.begin(), previous.end());
            for (auto& cost : local) {
                // cost -= min;
                cost = static_cast<accumulated_cost_value_t>(cost - min);
            }
        }
    }

    return accumulated_costs;
}

} // namespace

namespace img {

// returns a disparity image from two rectified grayscale images
// left image is the base image
ImageGray<std::uint8_t>
semi_global_matching(const ImageGray<std::uint8_t>& left, const ImageGray<std::uint8_t>& right) {

    // const auto costs = calculate_costs(left, right, pixelwise_absolute_difference);
    const auto costs = calculate_costs(left, right, sum_of_absolute_differences_3);

    const auto accumulated_costs_x1_y0 = accumulate_costs_direction_x1_y0(left, costs);

    // ImageGray<std::uint8_t> min_cost_view(left.width, left.height);
    // std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(naive_disparity.data),
    //                [](const costs_vector_t& cv){
    //                    return PixelGray<std::uint8_t>{ *std::min_element(std::begin(cv), std::end(cv)) };
    //                });

    static_assert(std::numeric_limits<std::uint8_t>::max() >= MAX_DISPARITY, "disparity values might fit into the image");
    ImageGray<std::uint8_t> naive_disparity(left.width, left.height);
    std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(naive_disparity.data),
                [](const costs_vector_t& cv){
                    const costs_vector_t::const_iterator result = std::min_element(std::begin(cv), std::end(cv));
                    return PixelGray<std::uint8_t>{ static_cast<std::uint8_t>(std::distance(std::begin(cv), result)) };
                });
    static_assert(MAX_DISPARITY == 64, "can't scale by shifting two bits"); for (auto& pixel : naive_disparity.data) {pixel.value = static_cast<std::uint8_t>(pixel.value * 4);};

    ImageGray<std::uint8_t> acc_disparity(left.width, left.height);
    std::transform(std::cbegin(accumulated_costs_x1_y0.data), std::cend(accumulated_costs_x1_y0.data), std::begin(acc_disparity.data),
                [](const auto& cv){
                    const auto result = std::min_element(std::begin(cv), std::end(cv));
                    return PixelGray<std::uint8_t>{ static_cast<std::uint8_t>(std::distance(std::begin(cv), result)) };
                });
    static_assert(MAX_DISPARITY == 64, "can't scale by shifting two bits"); for (auto& pixel : acc_disparity.data) {pixel.value = static_cast<std::uint8_t>(pixel.value * 4);};

    return acc_disparity;
}
} // namespace img
