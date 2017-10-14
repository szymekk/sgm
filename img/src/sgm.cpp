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

std::uint8_t abs_diff(const std::uint8_t a, const std::uint8_t b) {
    return static_cast<std::uint8_t>(a < b ? b - a : a - b);
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
    const std::size_t r_max = std::clamp(row + n, 0uL, left.height - 1);

    const std::size_t c_min = (col >= n) ? col - n : 0;
    const std::size_t c_max = std::clamp(col + n, 0uL, left.width - 1);

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

} // namespace

namespace img {

// returns a disparity image from two rectified grayscale images
// left image is the base image
ImageGray<std::uint8_t>
semi_global_matching(const ImageGray<std::uint8_t>& left, const ImageGray<std::uint8_t>& right) {

    // const auto costs = calculate_costs(left, right, pixelwise_absolute_difference);
    const auto costs = calculate_costs(left, right, sum_of_absolute_differences_3);

    // ImageGray<std::uint8_t> min_cost_view(left.width, left.height);
    // std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(naive_disparity.data),
    //                [](const costs_vector_t& cv){
    //                    return PixelGray<std::uint8_t>{ *std::min_element(std::begin(cv), std::end(cv)) };
    //                });

    static_assert(std::numeric_limits<std::uint8_t>::max() >= MAX_DISPARITY);
    ImageGray<std::uint8_t> naive_disparity(left.width, left.height);
    std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(naive_disparity.data),
               [](const costs_vector_t& cv){
                   const costs_vector_t::const_iterator result = std::min_element(std::begin(cv), std::end(cv));
                   return PixelGray<std::uint8_t>{ static_cast<std::uint8_t>(std::distance(std::begin(cv), result)) };
               });
    static_assert(MAX_DISPARITY == 64); for (auto& pixel : naive_disparity.data) {pixel.value = static_cast<std::uint8_t>(pixel.value * 4);};

    return naive_disparity;
}
} // namespace img
