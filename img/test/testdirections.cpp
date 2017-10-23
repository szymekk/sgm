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

std::map<std::pair<int, int>, fun_t*> ind_to_fun;

template<int x, int y>
fun_t sgm_direction;

// returns a disparity image from two rectified grayscale images
// left image is the base image
template<int x, int y>
img::ImageGray<std::uint8_t>
sgm_direction(const img::ImageGray<std::uint8_t>& left,
              const img::Grid<img::cost_arr_t>& costs) {
    const auto accumulated_costs = img::accumulate_costs_direction<x, y>(left, costs);
    return img::create_disparity_view(accumulated_costs);
}
} // namespace

int main(int argc, const char* argv[]) {

    // ind_to_fun[{1, 0}] = &old_direction;

    ind_to_fun[{+1,  0}] = &sgm_direction<+1,  0>;
    ind_to_fun[{+1, +1}] = &sgm_direction<+1, +1>;
    ind_to_fun[{ 0, +1}] = &sgm_direction< 0, +1>;
    ind_to_fun[{-1, +1}] = &sgm_direction<-1, +1>;

    ind_to_fun[{-1,  0}] = &sgm_direction<-1,  0>;
    ind_to_fun[{-1, -1}] = &sgm_direction<-1, -1>;
    ind_to_fun[{ 0, -1}] = &sgm_direction< 0, -1>;
    ind_to_fun[{+1, -1}] = &sgm_direction<+1, -1>;

    if (argc >= 4) {
        const std::string left_filename  = argv[1];
        const std::string right_filename = argv[2];
        const std::string out_basename   = argv[3];

        if (auto maybe_left_rgb  = img::make_rgb_image_from_ppm(left_filename),
                 maybe_right_rgb = img::make_rgb_image_from_ppm(right_filename);
            maybe_right_rgb.has_value() && maybe_left_rgb.has_value()) {

            const auto& left_gray  = rgb_to_gray_image(maybe_left_rgb.value());
            const auto& right_gray = rgb_to_gray_image(maybe_right_rgb.value());

            const auto costs = img::calculate_costs(left_gray, right_gray, img::sum_of_absolute_differences<3>);

            for (const auto [index, fun] : ind_to_fun) {
                const auto [x, y] = index;
                const auto& disparity_img = fun(left_gray, costs);
                const auto& max = std::max_element(std::begin(disparity_img.data), std::end(disparity_img.data),
                                                   [](const auto& px1, const auto& px2){return px1.value < px2.value;});
                std::cout << "max pixel for direction {x: " << x << ", y: " << y << "}: " << +max->value << "\n";
                std::string out_filename = out_basename + "_dir_" + std::to_string(x) + "_" + std::to_string(y) + ".pgm";
                std::cout << "saving disparity image to: " << out_filename << "\n";
                img::write_gray_image_to_pgm(disparity_img, out_filename);
            }
        } else {
            std::cerr << "cannot read PPM images properly \n";
            return 1;
        }

    }

    return 0;
}
