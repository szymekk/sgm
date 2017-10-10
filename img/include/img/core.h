#ifndef IMG_CORE_H
#define IMG_CORE_H

#include <cassert>
#include <vector>

namespace img {

    template <typename PixelT>
    struct Grid {
        Grid(const std::size_t cols, const std::size_t rows)
                : width(cols), height(rows), data(width*height)
        {}

        PixelT get(const std::size_t col, const std::size_t row) {
            assert(col < width);
            assert(row < height);
            return data.at(col + width*row);
        }

        std::size_t width;
        std::size_t height;
        std::vector<PixelT> data;
    };

    template <typename ChannelT>
    struct PixelGrey {
        ChannelT value;
    };

    template <typename ChannelT>
    struct PixelRGB {
        ChannelT red;
        ChannelT green;
        ChannelT blue;
    };

    template <typename ChannelT>
    using ImageGrey = Grid<PixelGrey<ChannelT>>;

    template <typename ChannelT>
    using ImageRGB = Grid<PixelRGB<ChannelT>>;

} // namespace img

#endif //IMG_CORE_H

