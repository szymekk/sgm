#include <iostream>
#include <fstream>
#include <cstdint>
#include <c++/7.2.0/cstring>

#include "img/core.h"
#include "img/fileio.h"

int main(int argc, const char* argv[]) {

    using ImageRGB = img::ImageRGB<std::uint8_t>;
    ImageRGB img_8bit(3, 2);
    img_8bit.data.front() = {'b','e','g'};
    img_8bit.data[1] = {'a','b','c'};
    img_8bit.data.back() = {'e','n','d'};

    for(size_t r = 0; r < img_8bit.height; ++r) {
        std::cout << r << ":\t";
        for(size_t c = 0; c < img_8bit.width; ++c) {
            const auto& cell = img_8bit.get(c, r);
            std::cout << "{";
            std::cout << cell.red   << ", ";
            std::cout << cell.green << ", ";
            std::cout << cell.blue  << "}\t";
        }
        std::cout << "\n";
    }

    if (argc >= 2) {
        const std::string in_filename = argv[1];
        if (const auto maybe_image_RGB = img::make_rgb_image_from_ppm(in_filename)) {
            auto image_RGB = maybe_image_RGB.value();
            for (const auto& pixel : image_RGB.data) {
                std::cout << "{";
                std::cout << +pixel.red   << ", "; //plus to display integer value
                std::cout << +pixel.green << ", ";
                std::cout << +pixel.blue  << "}, ";
            }
            std::cout << "\n";

            image_RGB.data[0].red   = 255;
            image_RGB.data[0].green = 0;
            image_RGB.data[0].blue  = 0;

            image_RGB.data[1] = {255, 0, 0};

            for (const auto& pixel : image_RGB.data) {
                std::cout << "{";
                std::cout << +pixel.red   << ", "; //plus to display integer value
                std::cout << +pixel.green << ", ";
                std::cout << +pixel.blue  << "}, ";
            }
            std::cout << "\n";
            if(argc == 3) {
                const std::string out_filename = argv[2];
                std::cout << "saving output to: " << out_filename << "\n";
                img::write_rgb_image_to_ppm(image_RGB, out_filename);
            }
        } else {
            std::cerr << "cannot read image properly \n";
        }
    }

    return 0;
}
