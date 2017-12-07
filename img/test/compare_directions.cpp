#include <iostream>
#include <fstream>
#include <map>
#include <iomanip>

#include "img/median.h"
#include "img/fileio.h"
#include "img/sgm.h"
#include "img/eval.h"

namespace {

const std::uint8_t BAD_PIXEL_THRESHOLD_ONE_PX = 4; // one pixel disparity on scaled disparity map
const std::uint8_t BAD_PIXEL_THRESHOLD_TWO_PX = 8; // two pixel disparity on scaled disparity map

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
        const std::string disparity_file = argv[3];

        std::ostream& os = std::cout;
        os << "L: " << left_filename << "\n";
        os << "R: " << right_filename << "\n";
        os << "T: " << disparity_file << "\n";

        const auto maybe_disparity = img::make_gray_image_from_pgm(disparity_file);
        if (const auto maybe_left_rgb   = img::make_rgb_image_from_ppm(left_filename),
                       maybe_right_rgb  = img::make_rgb_image_from_ppm(right_filename);
            maybe_right_rgb.has_value() && maybe_left_rgb.has_value() && maybe_disparity.has_value()) {

            const auto& left_gray  = rgb_to_gray_image(maybe_left_rgb.value());
            const auto& right_gray = rgb_to_gray_image(maybe_right_rgb.value());
            const auto& disparity = maybe_disparity.value();

//            const auto costs = img::calculate_costs(left_gray, right_gray, img::sum_of_absolute_differences<3>);
//            const auto costs = img::calculate_costs(left_gray, right_gray, img::pixelwise_absolute_difference);
            const auto costs = img::calculate_costs(left_gray, right_gray, img::truncated_pixelwise_absolute_difference);
//            const auto costs = img::calculate_costs(left_gray, right_gray, img::rank_transform_based_cost<7>);

            const auto& naive_disparity = img::create_disparity_view(costs);

            int accumulated_paths = 0;
            const auto compare_and_log = [disparity, &os](const auto& computed){
                const auto rms = img::root_mean_square_error(computed, disparity);
                const auto bad_percentage_one = img::percentage_of_bad_pixels(computed, disparity, BAD_PIXEL_THRESHOLD_ONE_PX);
                const auto bad_percentage_two = img::percentage_of_bad_pixels(computed, disparity, BAD_PIXEL_THRESHOLD_TWO_PX);
                os << std::setw(8) << rms << ", "
                   << std::setw(8) << std::left << bad_percentage_one << ", "
                   << std::setw(8) << std::left << bad_percentage_two;
            };

            const auto with_median = [&accumulated_paths, &os, &compare_and_log](const auto& computed){
                os << accumulated_paths++ << ", ";
                compare_and_log(computed);
                const auto filtered = img::median(computed, img::BoundaryTreatment::copy_from_input);
                os << "\tmedian filtered: ";
                compare_and_log(filtered);
                os << "\n";
            };

            os << "[direction]  " << ", RMS" << ", bad percentage (1px)" << ", bad percentage (2px)" << "\n";
            os << "[raw costs] ";
            with_median(naive_disparity);

            img::Grid<img::acc_cost_arr_t> total_costs(left_gray.width, left_gray.height);
            for (const auto [direction, path_function] : path_functions) {
                const auto [x, y] = direction;
                const auto& path_costs = path_function(left_gray, costs);
                os << std::showpos << "[dir_" << x << "_" << y << "] " << std::noshowpos;

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
                with_median(total_disparity);
            }
        } else {
            std::cerr << "cannot read images properly \n";
            return 1;
        }

    }

    return 0;
}
