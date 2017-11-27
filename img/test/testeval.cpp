#include <iostream>
#include <fstream>

#include "img/eval.h"
#include "img/fileio.h"

namespace {
const std::uint8_t BAD_PIXEL_THRESHOLD = 4; // one pixel disparity on scaled disparity map
} // namespace

int main(int argc, const char* argv[]) {

    if (argc >= 3) {
        const std::string first_filename  = argv[1];
        const std::string second_filename = argv[2];

        if (auto maybe_first_gray  = img::make_gray_image_from_pgm(first_filename),
                maybe_second_gray = img::make_gray_image_from_pgm(second_filename);
        maybe_first_gray.has_value() && maybe_second_gray.has_value()) {

            const auto& first_gray  = maybe_first_gray.value();
            const auto& second_gray = maybe_second_gray.value();

            const auto rms = img::root_mean_square_error(first_gray, second_gray);
            const auto bad_percentage = img::percentage_of_bad_pixels(first_gray, second_gray, BAD_PIXEL_THRESHOLD);

            std::cout << "rms: " << rms << " bad percentage: " << bad_percentage << "\n";
        } else {
            std::cerr << "cannot read PGM images properly \n";
            return 1;
        }
    } else {
        std::cerr << "provide 2 arguments\n";
        return 2;
    }

    return 0;
}
