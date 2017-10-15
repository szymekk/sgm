#include <iostream>
#include <fstream>
//#include <cstdint>

#include "img/core.h"
#include "img/fileio.h"
#include "img/sgm.h"

int main(int argc, const char* argv[]) {

    if (argc >= 3) {
        const std::string left_filename  = argv[1];
        const std::string right_filename = argv[2];
        if (auto maybe_left_rgb  = img::make_rgb_image_from_ppm(left_filename),
                 maybe_right_rgb = img::make_rgb_image_from_ppm(right_filename);
            maybe_right_rgb.has_value() && maybe_left_rgb.has_value()) {

            const auto& left_gray  = rgb_to_gray_image(maybe_left_rgb.value());
            const auto& right_gray = rgb_to_gray_image(maybe_right_rgb.value());

            const auto& disparity_img = img::semi_global_matching(left_gray, right_gray);
            const auto& max = std::max_element(std::begin(disparity_img.data), std::end(disparity_img.data),
                                               [](const auto& px1, const auto& px2){return px1.value < px2.value;});
            std::cout << "max pixel: " << +max->value << "\n";

            if(argc == 4) {
                const std::string out_filename = argv[3];
                std::cout << "saving disparity image to: " << out_filename << "\n";
                img::write_gray_image_to_pgm(disparity_img, out_filename);

                const std::string left_gray_outfile = left_filename + "_gray.pgm";
                const std::string right_gray_outfile = right_filename + "_gray.pgm";
                std::cout << "saving left grayscale image to: " << left_gray_outfile << "\n";
                std::cout << "saving right grayscale image to: " << right_gray_outfile << "\n";
                img::write_gray_image_to_pgm(left_gray, left_gray_outfile);
                img::write_gray_image_to_pgm(right_gray, right_gray_outfile);
            }

        } else {
            std::cerr << "cannot read PPM images properly \n";
            return 1;
        }

    }

    return 0;
}
