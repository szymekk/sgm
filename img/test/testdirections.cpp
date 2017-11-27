#include <iostream>
#include <fstream>
//#include <cstdint>
#include <map>

#include "img/core.h"
#include "img/fileio.h"
#include "img/sgm.h"

namespace {

using fun_t =
img::ImageGray<std::uint8_t>(
    const img::ImageGray<std::uint8_t>& left,
    const img::Grid<img::cost_arr_t>& costs);

using acc_fun_t =
img::Grid<img::acc_cost_arr_t>(
        const img::ImageGray<std::uint8_t>& left,
        const img::Grid<img::cost_arr_t>& costs);

using direction_t = std::pair<int, int>;

constexpr std::array<std::pair<direction_t, acc_fun_t*>, 8> path_functions = { {
        {{+1,  0}, &img::accumulate_costs_direction<+1,  0>},
        {{+1, +1}, &img::accumulate_costs_direction<+1, +1>},
        {{ 0, +1}, &img::accumulate_costs_direction< 0, +1>},
        {{-1, +1}, &img::accumulate_costs_direction<-1, +1>},

        {{-1,  0}, &img::accumulate_costs_direction<-1,  0>},

        {{-1, -1}, &img::accumulate_costs_direction<-1, -1>},
        {{ 0, -1}, &img::accumulate_costs_direction< 0, -1>},
        {{+1, -1}, &img::accumulate_costs_direction<+1, -1>},
} };
} // namespace

int main(int argc, const char* argv[]) {

    if (argc >= 4) {
        const std::string left_filename  = argv[1];
        const std::string right_filename = argv[2];
        const std::string out_basename   = argv[3];

        if (auto maybe_left_rgb  = img::make_rgb_image_from_ppm(left_filename),
                 maybe_right_rgb = img::make_rgb_image_from_ppm(right_filename);
            maybe_right_rgb.has_value() && maybe_left_rgb.has_value()) {

            const auto& left_gray  = rgb_to_gray_image(maybe_left_rgb.value());
            const auto& right_gray = rgb_to_gray_image(maybe_right_rgb.value());

//            const auto costs = img::calculate_costs(left_gray, right_gray, img::sum_of_absolute_differences<3>);
//            const auto costs = img::calculate_costs(left_gray, right_gray, img::pixelwise_absolute_difference);
            const auto costs = img::calculate_costs(left_gray, right_gray, img::rank_transform_based_cost<7>);

            img::Grid<img::acc_cost_arr_t> total_costs(left_gray.width, left_gray.height);
            for (const auto [direction, path_function] : path_functions) {
                const auto [x, y] = direction;
                const auto& path_costs = path_function(left_gray, costs);
                const auto& path_disparity = img::create_disparity_view(path_costs);
                const auto& max = std::max_element(std::begin(path_disparity.data), std::end(path_disparity.data),
                                                   [](const auto& px1, const auto& px2){return px1.value < px2.value;});
                std::cout << "max pixel for direction {x: " << x << ", y: " << y << "}: " << +max->value << "\n";

                const std::string out_filename = out_basename + "_dir_" + std::to_string(x) + "_" + std::to_string(y) + ".pgm";
                std::cout << "saving disparity image to: " << out_filename << "\n";
                img::write_gray_image_to_pgm(path_disparity, out_filename);

                //total_costs += path_costs;
                auto& total_vector = total_costs.data;
                const auto& path_vector = path_costs.data;
                for (std::size_t px = 0; px < total_vector.size(); ++px) {
                    auto& total_px = total_vector[px];
                    const auto& path_px = path_vector[px];
                    std::transform(total_px.begin(), total_px.end(), path_px.begin(),
                                   total_px.begin(), std::plus<>());
                }

                const auto& total_disparity = img::create_disparity_view(total_costs);
                const std::string sum_out_filename = out_basename + "_sum_after_" + std::to_string(x) + "_" + std::to_string(y) + ".pgm";
                std::cout << "saving sum disparity image to: " << sum_out_filename << "\n";
                img::write_gray_image_to_pgm(total_disparity, sum_out_filename);
            }
        } else {
            std::cerr << "cannot read PPM images properly \n";
            return 1;
        }

    }

    return 0;
}
