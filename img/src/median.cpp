#include <array>
#include <c++/7.2.0/iostream>
#include "img/median.h"

namespace {
const int WINDOW_SIZE = 3;

auto get_median_value(std::array<std::uint8_t, WINDOW_SIZE * WINDOW_SIZE>& window) {
    auto mid_iter = std::begin(window) + std::size(window) / 2;
    std::nth_element(std::begin(window), mid_iter, std::end(window));
    return *mid_iter;
}

}

namespace img {

// 3x3 median
ImageGray<std::uint8_t>
median(const ImageGray<std::uint8_t>& image, const BoundaryTreatment boundary_treatment) {
    const auto height = image.height;
    const auto width = image.width;
    ImageGray<std::uint8_t> result_image(width, height);

    static_assert(1 == WINDOW_SIZE % 2, "even window size is not supported");
    constexpr int OFFSET = WINDOW_SIZE / 2;
    for (auto row = OFFSET; row < static_cast<int>(height) - OFFSET; ++row) {
        for (auto col = OFFSET ; col < static_cast<int>(width) - OFFSET; ++col) {
            std::array<std::uint8_t, WINDOW_SIZE * WINDOW_SIZE> window{}; // zero initialized
            std::size_t counter = 0;
            for (auto y_offset = -OFFSET; y_offset <= OFFSET; ++y_offset) {
                for (auto x_offset = -OFFSET; x_offset <= OFFSET; ++x_offset) {
                    const auto x = static_cast<std::size_t>(col + x_offset);
                    const auto y = static_cast<std::size_t>(row + y_offset);
                    window[counter++] = image.get(x, y).value;
                }
            }
            //get median value
            const auto median = get_median_value(window);
            result_image.get(static_cast<std::size_t>(col),
                             static_cast<std::size_t>(row)).value = median;
        }
    }
    switch (boundary_treatment) {
        case BoundaryTreatment::set_to_zero:
            break; // result array element are initialized to zero
        case BoundaryTreatment::copy_from_input: {
            const auto last_row = height - 1;
            const auto last_col = width - 1;
            const auto copy_pixel_at_position = [&result_image, &image](const std::size_t c, const std::size_t r) {
                result_image.get(c, r) = image.get(c, r);
            };
            for (std::size_t col = 0; col < width; ++col) {
                copy_pixel_at_position(col, 0);
                copy_pixel_at_position(col, last_row);
            }
            for (std::size_t row = 1; row < height - 1; ++row) {
                copy_pixel_at_position(0, row);
                copy_pixel_at_position(last_col, row);
            }
            break;
        }
        case BoundaryTreatment::zero_pad_input: {
            const auto zero_padded_median = [&](const std::size_t c, const std::size_t r){
                std::array<std::uint8_t, WINDOW_SIZE * WINDOW_SIZE> window{}; // zero initialized
                std::size_t counter = 0;
                for (auto y_offset = -OFFSET; y_offset <= OFFSET; ++y_offset) {
                    for (auto x_offset = -OFFSET; x_offset <= OFFSET; ++x_offset) {
                        const int x = static_cast<int>(c) + x_offset;
                        const int y = static_cast<int>(r) + y_offset;
                        const bool valid_x = x >= 0 && x < static_cast<int>(width);
                        const bool valid_y = y >= 0 && y < static_cast<int>(height);
                        const bool valid_position = valid_x && valid_y;
                        if (valid_position) {
                            const auto x_pos = static_cast<std::size_t>(x);
                            const auto y_pos = static_cast<std::size_t>(y);
                            window[counter] = image.get(x_pos, y_pos).value;
                        }
                        ++counter;
                    }
                }
                //get median value
                const auto median = get_median_value(window);
                result_image.get(static_cast<std::size_t>(c),
                                 static_cast<std::size_t>(r)).value = median;
            };
            const auto last_row = height - 1;
            const auto last_col = width - 1;
            for (std::size_t col = 0; col < width; ++col) {
                zero_padded_median(col, 0);
                zero_padded_median(col, last_row);
            }
            for (std::size_t row = 1; row < height - 1; ++row) {
                zero_padded_median(0, row);
                zero_padded_median(last_col, row);
            }
            break;
        }
    }
    return result_image;
}

}
