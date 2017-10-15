#ifndef IMG_FILEIO_H
#define IMG_FILEIO_H

#include <optional>
#include <istream>

#include "img/core.h"

namespace img {

std::optional<ImageRGB<std::uint8_t>> make_rgb_image_from_ppm(const std::string& filename);
std::optional<ImageGray<std::uint8_t>> make_gray_image_from_pgm(const std::string& filename);

void write_rgb_image_to_ppm(const ImageRGB < std::uint8_t >& image, const std::string& filename);
void write_gray_image_to_pgm(const ImageGray < std::uint8_t >& image, const std::string& filename);

} // namespace img

#endif //IMG_FILEIO_H

