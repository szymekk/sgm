#include <array>
#include <c++/7.2.0/iostream>
#include "img/median.h"

namespace {
const int WINDOW_SIZE = 3;
}

namespace img {

// 3x3 median
ImageGray<std::uint8_t> median(const ImageGray<std::uint8_t>& image) {
    const auto height = image.height;
    const auto width = image.width;
    ImageGray<std::uint8_t> result_image(width, height);

    static_assert(1 == WINDOW_SIZE % 2, "even window size is not supported");
    constexpr int OFFSET = WINDOW_SIZE / 2;
    for (auto row = OFFSET; row < static_cast<int>(height) - OFFSET; ++row) {
        for (auto col = OFFSET ; col < static_cast<int>(width) - OFFSET; ++col) {
            // std::cerr << "processing column: " << col << "\n";
            std::array<std::uint8_t, WINDOW_SIZE * WINDOW_SIZE> window;
            std::size_t counter = 0;
            for (auto y_offset = -OFFSET; y_offset <= OFFSET; ++y_offset) {
                for (auto x_offset = -OFFSET; x_offset <= OFFSET; ++x_offset) {
                    const auto x_pos = static_cast<std::size_t>(col + x_offset);
//                    const auto x_pos = (col + x_offset);
                    const auto y_pos = static_cast<std::size_t>(row + y_offset);
                    window[counter++] = image.get(x_pos, y_pos).value;
//                    if (row == 5 && col == 7) {
//                        std::cout << +image.get(col + x_offset, row + y_offset).value << " ; ";
//                    }
                }
            }
            if (row == 5 && col == 7) {
                std::cout << -OFFSET << "\n";
                for (const auto el : window) {
                    std::cout << " , " << +el;
                }
                std::cout << "\n";
            }

            auto mid_iter = std::begin(window) + std::size(window) / 2;
            std::nth_element(std::begin(window), mid_iter, std::end(window));
            result_image.get(static_cast<std::size_t>(col),static_cast<std::size_t>(row)).value = *mid_iter;
        }
    }
    return result_image;
}

// 3x3 median
//ImageGray<std::uint8_t> median(const ImageGray<std::uint8_t>& image) {
//    const auto height = image.height;
//    const auto width = image.width;
//    ImageGray<std::uint8_t> result_image(width, height);
//
//    static_assert(1 == WINDOW_SIZE % 2, "even window size is not supported");
//    constexpr std::size_t OFFSET = WINDOW_SIZE / 2;
//    for (auto row = OFFSET; row < height - OFFSET; ++row) {
//        for (auto col = OFFSET ; col < width - OFFSET; ++col) {
//            // std::cerr << "processing column: " << col << "\n";
//            std::array<std::uint8_t, WINDOW_SIZE * WINDOW_SIZE> window;
//            std::size_t counter = 0;
//            for (auto y_offset = static_cast<int>(-OFFSET); y_offset <= static_cast<int>(OFFSET); ++y_offset) {
//                for (auto x_offset = static_cast<int>(-OFFSET); x_offset <= static_cast<int>(OFFSET); ++x_offset) {
//                    const auto x_pos = static_cast<std::size_t>(col + x_offset);
////                    const auto x_pos = (col + x_offset);
//                    const auto y_pos = static_cast<std::size_t>(row + y_offset);
//                    window[counter++] = image.get(x_pos, y_pos).value;
////                    if (row == 5 && col == 7) {
////                        std::cout << +image.get(col + x_offset, row + y_offset).value << " ; ";
////                    }
//                }
//            }
//            if (row == 5 && col == 7) {
//                std::cout << -OFFSET << "\n";
//                for (const auto el : window) {
//                    std::cout << " , " << +el;
//                }
//                std::cout << "\n";
//            }
//
//            auto mid_iter = std::begin(window) + std::size(window) / 2;
//            std::nth_element(std::begin(window), mid_iter, std::end(window));
//            result_image.get(col, row).value = *mid_iter;
//        }
//    }
//    return result_image;
//}

}
