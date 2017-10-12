#include <iostream>
#include <fstream>
#include <cstdint>

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

    std::optional<img::ImageRGB<std::uint8_t>>  maybe_image_RGB;
    std::optional<img::ImageGray<std::uint8_t>> maybe_image_gray_from_RGB;
    std::optional<img::ImageGray<std::uint8_t>> maybe_image_gray;

    if (argc >= 2) {
        const std::string in_basename = argv[1];
        const std::string in_file_ppm = in_basename + ".ppm";
        if (maybe_image_RGB = img::make_rgb_image_from_ppm(in_file_ppm)) {
            auto& image_RGB = maybe_image_RGB.value();
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

            image_RGB.data[1] = {255, 255, 255};

            maybe_image_gray_from_RGB = rgb_to_gray_image(image_RGB);
            for (const auto& pixel : maybe_image_gray_from_RGB->data) {
                std::cout << +pixel.value << ", ";
            }
            std::cout << "\n";

        } else {
            std::cerr << "cannot read PPM image properly \n";
        }

        const std::string in_file_pgm = in_basename + ".pgm";
        if (maybe_image_gray = img::make_gray_image_from_pgm(in_file_pgm)) {
            auto image_gray = maybe_image_gray.value();
            for (const auto& pixel : image_gray.data) {
                std::cout << +pixel.value << ", ";
            }
            std::cout << "\n";

        } else {
            std::cerr << "cannot read PGM image properly \n";
        }

        if(argc == 3) {
            const std::string out_basename = argv[2];

            const std::string out_file_ppm = out_basename + ".ppm";
            if (maybe_image_RGB) {
                std::cout << "saving RGB to: " << out_file_ppm << "\n";
                img::write_rgb_image_to_ppm(maybe_image_RGB.value(), out_file_ppm);
            }

            const std::string out_file_modified_rgb_pgm = out_basename + ".modified" + ".pgm";
            if (maybe_image_gray_from_RGB) {
                std::cout << "saving modified RGB as grayscale to: " << out_file_modified_rgb_pgm << "\n";
                img::write_gray_image_to_pgm(maybe_image_gray_from_RGB.value(), out_file_modified_rgb_pgm);
            }

            const std::string out_file_pgm = out_basename + ".pgm";
            if (maybe_image_gray) {
                std::cout << "saving grayscale to: " << out_file_pgm << "\n";
                img::write_gray_image_to_pgm(maybe_image_gray.value(), out_file_pgm);
            }
        }
    }

    return 0;
}
