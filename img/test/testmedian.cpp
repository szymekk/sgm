#include <iostream>
#include <fstream>

#include "img/median.h"
#include "img/fileio.h"

int main(int argc, const char* argv[]) {

    if (argc >= 3) {
        const std::string in_filename = argv[1];
        const std::string out_filename = argv[2];

        if (auto maybe_gray  = img::make_gray_image_from_pgm(in_filename);
            maybe_gray.has_value()) {

            const auto& gray  = maybe_gray.value();
            const auto filtered_image = img::median(gray, img::BoundaryTreatment::copy_from_input);

            img::write_gray_image_to_pgm(filtered_image, out_filename);
        } else {
            std::cerr << "cannot read PGM image properly \n";
            return 1;
        }
    } else {
        std::cerr << "provide 2 arguments\n";
        return 2;
    }

    return 0;
}
