#include <algorithm>
#include <iostream>

#include "img/fileio.h"
#include "img/sgm.h"

namespace {

const img::acc_cost_t PENALTY_1 = 15;
const img::acc_cost_t PENALTY_2 = 100;
const bool DIVIDE_PENALTY_2 = true;

template<typename T>
T abs_diff(const T a, const T b) {
    return static_cast<T>(a < b ? b - a : a - b);
}

using img::acc_cost_t;
using img::acc_cost_arr_t;
using img::MAX_DISPARITY;

acc_cost_arr_t
compute_additional_cost(
        const acc_cost_arr_t& previous,
        const std::uint8_t intensity_change) {

    const auto penalty_2 = [&]{
        if constexpr (DIVIDE_PENALTY_2) {
            const auto penalty_2_tentative =
                    static_cast<acc_cost_t>(intensity_change ? PENALTY_2 / intensity_change : PENALTY_2);
            return std::max(PENALTY_1, penalty_2_tentative);
        } else {
            return PENALTY_2;
        }
    }();

    acc_cost_arr_t additional_costs;
    for (std::size_t d = 0; d < MAX_DISPARITY; ++d) {
        auto additional_cost = std::numeric_limits<acc_cost_t>::max();
        for (std::size_t d_p = 0; d_p < MAX_DISPARITY; ++d_p) {
            const auto diff = abs_diff(d_p, d);
            if (0 == diff) {
                additional_cost = std::min(additional_cost, previous[d_p]);
            } else if (1 == diff) {
                additional_cost = std::min(additional_cost, static_cast<acc_cost_t>(previous[d_p] + PENALTY_1));
            } else {
                additional_cost = std::min(additional_cost, static_cast<acc_cost_t>(previous[d_p] + penalty_2));
            }
        }
        additional_costs[d] = additional_cost;
    }
    return additional_costs;
}

} // namespace

namespace img {

[[maybe_unused]]
cost_t
pixelwise_absolute_difference(const img::ImageGray<std::uint8_t>& left, const img::ImageGray<std::uint8_t>& right,
                              const std::size_t row, const std::size_t col, const std::size_t disparity) {
    if (col < disparity) {
        return INVALID_COST;
    } else {
        return abs_diff(left.get(col, row).value, right.get(col - disparity, row).value);
    }
}

template<std::size_t WINDOW_SIZE>
cost_t sum_of_absolute_differences(const img::ImageGray<std::uint8_t>& left, const img::ImageGray<std::uint8_t>& right,
                                   const std::size_t row, const std::size_t col, const std::size_t disparity) {
    static_assert(WINDOW_SIZE > 0, "window size should be positive");
    static_assert(WINDOW_SIZE % 2 == 1, "window should be symmetrical");

    constexpr auto n = (WINDOW_SIZE - 1) / 2;
    unsigned int sum = 0;
    std::size_t valid_pixels = 0;

    static_assert(
            std::numeric_limits<decltype(sum)>::max() >=
            WINDOW_SIZE * WINDOW_SIZE * std::numeric_limits<cost_t>::max());


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
    return static_cast<cost_t>(sum / valid_pixels);
}

// todo: add different cost functions
img::Grid<cost_arr_t>
calculate_costs(const img::ImageGray<std::uint8_t>& left, const img::ImageGray<std::uint8_t>& right,
                cost_function_t cost_function) {

    img::Grid<cost_arr_t> costs(left.width, left.height);
    for (std::size_t row = 0; row < left.height; ++row) {
        // std::cerr << "processing row: " << row << "\n";
        for (std::size_t col = 0; col < left.width; ++col) {
            // std::cerr << "processing column: " << col << "\n";
            for (std::size_t d = 0; d < MAX_DISPARITY; ++d) {
                // std::cerr << "processing disparity: " << d << "\n";
                if (col < d) {
                    costs.get(col, row)[d] = INVALID_COST;
                } else {
                    // std::cerr << "c: " << col << ", r: " << row << ", d: " << d << "...";
                    const cost_t calculated = cost_function(left, right, row, col, d);
                    // std::cerr << " calculated: " << +calculated << "...";
                    costs.get(col, row)[d] = calculated;
                    // std::cerr << "assigned!\n";
                }
            }
        }
    }
    return costs;
}

// accumulated cost at location (c, r) is equal to the sum of three terms:
// 1) initial cost at location (c, r)
// 2) additional cost depending on:
//    a) accumulated cost at previous location (c - 1, r),
//    b) the absolute difference of intensity values at the current and previous locations
// 3) negated value of the minimal element of term 2a
img::Grid<acc_cost_arr_t>
accumulate_costs_direction_x1_y0(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs) {

    img::Grid<acc_cost_arr_t> accumulated_costs(costs.width, costs.height);
    // (1) initialize accumulated costs with values from costs
    std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(accumulated_costs.data),
                   [](const cost_arr_t& cv) {
                       acc_cost_arr_t acc_cv;
                       std::copy(cv.begin(), cv.end(), std::begin(acc_cv));
                       return acc_cv;
                   });

    for (std::size_t r = 0; r < accumulated_costs.height; ++r) {
        for (std::size_t c = 1; c < accumulated_costs.width; ++c) {
            // (2)
            const auto intensity_change = abs_diff(left.get(c, r).value, left.get(c - 1, r).value);
            const auto& previous = accumulated_costs.get(c - 1, r);
            const auto additional_costs =
                    compute_additional_cost(previous, intensity_change);
            // accumulated_costs.get(c, r) += additional_costs;
            auto& local = accumulated_costs.get(c, r);
            std::transform(local.begin(), local.end(), additional_costs.begin(),
                           local.begin(), std::plus<>());

            // (3) substract min_element of previous from each element of local
            const auto min = *std::min_element(previous.begin(), previous.end());
            for (auto& cost : local) {
                // cost -= min;
                cost = static_cast<acc_cost_t>(cost - min);
            }
        }
    }

    return accumulated_costs;
}

// accumulated cost at location (c, r) is equal to the sum of three terms:
// 1) initial cost at location (c, r)
// 2) additional cost depending on:
//    a) accumulated cost at previous location (c - 1, r),
//    b) the absolute difference of intensity values at the current and previous locations
// 3) negated value of the minimal element of term 2a
template<int dir_x, int dir_y>
img::Grid<acc_cost_arr_t>
accumulate_costs_direction(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs) {
    static_assert(dir_y != 0 || dir_x != 0, "there should be some movement");

    img::Grid<acc_cost_arr_t> accumulated_costs(costs.width, costs.height);
    // (1) initialize accumulated costs with values from costs
    std::transform(std::cbegin(costs.data), std::cend(costs.data), std::begin(accumulated_costs.data),
                   [](const cost_arr_t& cv) {
                       acc_cost_arr_t acc_cv;
                       std::copy(cv.begin(), cv.end(), std::begin(acc_cv));
                       return acc_cv;
                   });

    const auto compute_accumulated_cost_at_position =
        [&left, &accumulated_costs](const auto col, const auto row) {
            const auto r_curr = static_cast<std::size_t>(row);
            const auto c_curr = static_cast<std::size_t>(col);

            const auto r_prev = static_cast<std::size_t>(static_cast<int>(row) - dir_y);
            const auto c_prev = static_cast<std::size_t>(static_cast<int>(col) - dir_x);

            const auto intensity_change = abs_diff(left.get(c_curr, r_curr).value,
                                                   left.get(c_prev, r_prev).value);

            const auto& previous = accumulated_costs.get(c_prev, r_prev);
            // (2)
            const auto additional_costs =
                    compute_additional_cost(previous, intensity_change);
            // accumulated_costs.get(c_curr, r_curr) += additional_costs;
            auto& local = accumulated_costs.get(c_curr, r_curr);
            std::transform(local.begin(), local.end(), additional_costs.begin(),
                           local.begin(), std::plus<>());

            // (3) substract min_element of previous from each element of local
            const auto min = *std::min_element(previous.begin(), previous.end());
            for (auto& cost : local) {
                // cost -= min;
                cost = static_cast<acc_cost_t>(cost - min);
        }
    };


    constexpr int r_first = (dir_y > 0) ? 1 : 0;
    constexpr int c_first = (dir_x > 0) ? 1 : 0;

    constexpr std::size_t r_offset = (dir_y < 0) ? 1 : 0;
    constexpr std::size_t c_offset = (dir_x < 0) ? 1 : 0;

    constexpr auto get_last_index = [](auto size, auto offset) -> int {
        return static_cast<int>(size) - 1 - static_cast<int>(offset);
    };

    const auto r_last = get_last_index(accumulated_costs.height, r_offset);
    const auto c_last = get_last_index(accumulated_costs.width, c_offset);

    constexpr bool forward = dir_y > 0 || (dir_y == 0 && dir_x > 0);
    if constexpr (forward) {
        for (int r = r_first; r <= r_last; ++r) {
            for (int c = c_first; c <= c_last; ++c) {
                compute_accumulated_cost_at_position(c, r);
            }
        }
    } else {
        for (int r = r_last; r >= r_first; --r) {
            for (int c = c_last; c >= c_first; --c) {
                compute_accumulated_cost_at_position(c, r);
            }
        }
    }

    return accumulated_costs;
}

template img::Grid<acc_cost_arr_t> accumulate_costs_direction<-1, -1>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);
template img::Grid<acc_cost_arr_t> accumulate_costs_direction<-1,  0>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);
template img::Grid<acc_cost_arr_t> accumulate_costs_direction<-1, +1>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);

template img::Grid<acc_cost_arr_t> accumulate_costs_direction< 0, -1>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);
template img::Grid<acc_cost_arr_t> accumulate_costs_direction< 0, +1>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);

template img::Grid<acc_cost_arr_t> accumulate_costs_direction<+1, -1>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);
template img::Grid<acc_cost_arr_t> accumulate_costs_direction<+1,  0>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);
template img::Grid<acc_cost_arr_t> accumulate_costs_direction<+1, +1>(const img::ImageGray<std::uint8_t>& left, const img::Grid<cost_arr_t>& costs);


// returns a disparity image from two rectified grayscale images
// left image is the base image
ImageGray<std::uint8_t>
semi_global_matching(const ImageGray<std::uint8_t>& left, const ImageGray<std::uint8_t>& right) {

    // const auto costs = calculate_costs(left, right, pixelwise_absolute_difference);
    const auto costs = calculate_costs(left, right, sum_of_absolute_differences<3>);
    const auto accumulated_costs_x1_y0 = accumulate_costs_direction_x1_y0(left, costs);

    //todo: accumulate costs along other directions and sum all costs

    return create_disparity_view(accumulated_costs_x1_y0);
}

} // namespace img
