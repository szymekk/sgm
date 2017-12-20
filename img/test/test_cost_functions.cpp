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

using name_t = std::string;

constexpr std::array<acc_fun_t*, 4> path_functions = { {
        &img::accumulate_costs_direction<+1,  0>,
        &img::accumulate_costs_direction<+1, +1>,
        &img::accumulate_costs_direction< 0, +1>,
        &img::accumulate_costs_direction<-1, +1>,

//        &img::accumulate_costs_direction<-1,  0>,
//
//        &img::accumulate_costs_direction<-1, -1>,
//        &img::accumulate_costs_direction< 0, -1>,
//        &img::accumulate_costs_direction<+1, -1>,
} };

const std::array<std::pair<std::string, img::cost_function_t*>, 6> cost_functions = { {
        {"sad_w7", &img::sum_of_absolute_differences<7>},
        {"zsad_w7", &img::zero_mean_sum_of_absolute_differences<7>},
        {"rank_w7", &img::rank_transform_based_cost<7>},
        {"census_w7", &img::census_transform_based_cost<7>},
        {"w1_trunc63", &img::truncated_pixelwise_absolute_difference},
        {"w1", &img::pixelwise_absolute_difference},
} };

} // namespace

int main(int argc, const char* argv[]) {

    if (argc >= 4) {
        const std::string left_filename  = argv[1];
        const std::string right_filename = argv[2];
        const std::string out_basename = argv[3];

        std::ostream& os = std::cout;
        os << "L: " << left_filename << "\n";
        os << "R: " << right_filename << "\n";

        if (const auto maybe_left_rgb   = img::make_rgb_image_from_ppm(left_filename),
                maybe_right_rgb  = img::make_rgb_image_from_ppm(right_filename);
        maybe_right_rgb.has_value() && maybe_left_rgb.has_value()) {

            const auto& left_gray  = rgb_to_gray_image(maybe_left_rgb.value());
            const auto& right_gray = rgb_to_gray_image(maybe_right_rgb.value());

            const auto sgm = [&left_gray, &right_gray](
                    img::cost_function_t *cost_function) -> img::ImageGray<std::uint8_t> {
                const auto costs = calculate_costs(left_gray, right_gray, cost_function);
                img::Grid<img::acc_cost_arr_t> total_costs(left_gray.width, left_gray.height);
                for (const auto path_function : path_functions) {
                    const auto& path_costs = path_function(left_gray, costs);

                    //total_costs += path_costs;
                    auto& total_vector = total_costs.data;
                    const auto& path_vector = path_costs.data;
                    for (std::size_t px = 0; px < total_vector.size(); ++px) {
                        auto& total_px = total_vector[px];
                        const auto& path_px = path_vector[px];
                        std::transform(total_px.begin(), total_px.end(), path_px.begin(),
                                       total_px.begin(), std::plus<>());
                    }
                }
                return img::create_disparity_view(total_costs);
            };

            img::Grid<img::acc_cost_arr_t> total_costs(left_gray.width, left_gray.height);
            for (const auto [cost_name, cost_function] : cost_functions) {
                const auto& computed_disparity = sgm(cost_function);
                const std::string out_filename = out_basename + "_4dir_" + cost_name + ".pgm";
                std::cout << "saving sum disparity image to: " << out_filename << "\n";
                img::write_gray_image_to_pgm(computed_disparity, out_filename);
            }
        } else {
            std::cerr << "cannot read images properly \n";
            return 1;
        }

    }

    return 0;
}
