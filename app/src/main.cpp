#include <iostream>
#include <cstdint>

#include "img/core.h"

int main() {

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

    return 0;
}
