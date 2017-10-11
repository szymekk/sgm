#include <iostream>
#include <fstream>

#include "img/fileio.h"

namespace {

void write_rgb_image_to_ppm_fstream(img::ImageRGB<std::uint8_t> &image, std::ofstream &output_file) {
    output_file << "P6\n";
    output_file << image.width << " " << image.height << "\n";
    output_file << "255\n"; // max value
    for (const auto &pixel : image.data) {
        output_file << static_cast<char>(pixel.red)
                    << static_cast<char>(pixel.green)
                    << static_cast<char>(pixel.blue);
    }
}

std::optional< img::ImageRGB< std::uint8_t > > make_rgb_image_from_ppm_fstream(std::istream &input_file) {
    //check file type
    if ('P' != input_file.get()) {
        std::cerr << "invalid header";
        return {};
    }
    if ('6' != input_file.get()) {
        std::cerr << "invalid header\n";
        return {};
    }

    //get size
    std::size_t width, height;
    input_file >> width >> height;

    if ('\n' != input_file.get()) {
        std::cerr << "invalid header\n";
        return {};
    }

    size_t max_value;
    input_file >> max_value;

    if ('\n' != input_file.get()) {
        std::cerr << "invalid header\n";
        return {};
    }

    img::ImageRGB<std::uint8_t> imageRGB(width, height);

    std::istreambuf_iterator<char> file_it(input_file), file_end;

    auto img_it = imageRGB.data.begin();
    auto img_end = imageRGB.data.cend();

    for (; img_it != img_end && file_it != file_end; ++img_it, ++file_it) {
        const auto r = static_cast<std::uint8_t>(*file_it);
        if (++file_it == file_end) {
            std::cerr << "file ended unexpectedly after r\n";
            return {};
        }

        const auto g = static_cast<std::uint8_t>(*file_it);
        if (++file_it == file_end) {
            std::cerr << "file ended unexpectedly after g\n";
            return {};
        }

        const auto b = static_cast<std::uint8_t>(*file_it);

        img_it->red   = r;
        img_it->green = g;
        img_it->blue  = b;
    }

    return imageRGB;
}

} // namespace

namespace img {

std::optional< ImageRGB< std::uint8_t > > make_rgb_image_from_ppm(const std::string& filename) {
    if (auto input_file_stream = std::ifstream(filename, std::ios::in | std::ios::binary)) {
        return make_rgb_image_from_ppm_fstream(input_file_stream);
    } else {
        std::cerr << "is_open(): " << input_file_stream.is_open() << "\n";
        std::cerr << "cannot read from file: " << filename << "\n";
        return {};
    }
}

void write_rgb_image_to_ppm(ImageRGB<std::uint8_t> &image, const std::string &filename) {
    if (auto output_file = std::ofstream(filename, std::ios::out | std::ios::binary)) {
        write_rgb_image_to_ppm_fstream(image, output_file);
        output_file.close();
    } else {
        std::cerr << "could not open file: " << filename << "for writing\n";
    }
}

} // namespace img

