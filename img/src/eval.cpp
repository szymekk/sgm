#include "img/eval.h"

#include <numeric> // std::inner_product
#include <functional> // std::plus
#include <cmath> // sqrt

namespace {

template<typename T>
T abs_diff(const T a, const T b) {
    return static_cast<T>(a < b ? b - a : a - b);
}

using accumulator_t = std::uint_fast32_t;

}

namespace img {

quality_metric_t
root_mean_square_error(const ImageGray<std::uint8_t>& img_first,
                       const ImageGray<std::uint8_t>& img_second) {
    assert(img_first.height == img_second.height && img_first.width == img_second.width);
    const auto first = img_first.data;
    const auto second = img_second.data;
    const auto element_count = first.size();

    const auto squared_error = [](const auto pixel_1, const auto pixel_2) {
        const auto difference = pixel_1.value - pixel_2.value;
        return static_cast<accumulator_t>(difference * difference);
    };

    const auto sum_of_squared_errors = static_cast<const accumulator_t>(
            std::inner_product(std::begin(first), std::end(first),
                               std::begin(second), 0,
                               std::plus<>(), squared_error));
    const auto mean_square_error = sum_of_squared_errors / element_count;
    return sqrt(static_cast<double>(mean_square_error));
}

quality_metric_t
percentage_of_bad_pixels(const img::ImageGray<std::uint8_t>& img_first,
                         const img::ImageGray<std::uint8_t>& img_second,
                         const std::uint8_t threshold) {
    assert(img_first.height == img_second.height && img_first.width == img_second.width);
    const auto first = img_first.data;
    const auto second = img_second.data;
    const auto element_count = first.size();

    const auto predicate = [threshold](const auto pixel_1, const auto pixel_2) {
        const auto absolute_difference = abs_diff(pixel_1.value, pixel_2.value);
        return absolute_difference > threshold;
    };

    const auto bad_pixel_count = static_cast<accumulator_t>(
            std::inner_product(std::begin(first), std::end(first),
                               std::begin(second), 0,
                               std::plus<>(), predicate));

    return static_cast<quality_metric_t>(bad_pixel_count)
           /
           static_cast<quality_metric_t>(element_count);
}

}
