#ifndef IMG_CORE_H
#define IMG_CORE_H

#include <cassert>
#include <vector>
#include <algorithm>

namespace img {

template <typename PixelT>
struct Grid {
    Grid(const std::size_t cols, const std::size_t rows)
            : width(cols), height(rows), data(width*height)
    {}

    PixelT& get(const std::size_t col, const std::size_t row) {
        assert(col < width);
        assert(row < height);
        return data.at(col + width*row);
    }

    const PixelT& get(const std::size_t col, const std::size_t row) const {
        assert(col < width);
        assert(row < height);
        return data.at(col + width*row);
    }

    std::size_t width;
    std::size_t height;
    std::vector<PixelT> data;
};

template <typename ChannelT>
struct PixelGray {
    ChannelT value;
};

template <typename ChannelT>
struct PixelRGB {
    ChannelT red;
    ChannelT green;
    ChannelT blue;
};

template <typename ChannelT>
using ImageGray = Grid<PixelGray<ChannelT>>;

template <typename ChannelT>
using ImageRGB = Grid<PixelRGB<ChannelT>>;

template <typename ChannelT>
ImageGray<ChannelT> rgb_to_gray_image(const ImageRGB<ChannelT>& rgb_image) {

    ImageGray<ChannelT> gray_image(rgb_image.width, rgb_image.height);

    auto rgb_to_gray_pixel = [](const PixelRGB<ChannelT>& pixel) {
        return PixelGray<ChannelT>{static_cast<ChannelT>((pixel.red + pixel.green + pixel.blue) / 3 )};
    };
    std::transform(rgb_image.data.cbegin(), rgb_image.data.cend(), gray_image.data.begin(),
            rgb_to_gray_pixel);

    return gray_image;
}

} // namespace img

#endif //IMG_CORE_H

