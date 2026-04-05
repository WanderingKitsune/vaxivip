#ifndef IMAGE_INFO_HPP
#define IMAGE_INFO_HPP

#include <cstdint>

/**
 * @brief Color depth in bits per color
 */
enum ColorDepth {
    COLOR_DEPTH_NONE = -1,
    COLOR_DEPTH_8 = 8,
    COLOR_DEPTH_10 = 10,
    COLOR_DEPTH_12 = 12
};

/**
 * @brief Image information
 */
class ImageInfo {
public:
    uint32_t width;
    uint32_t height;
    ColorDepth color_depth;
};

#endif
