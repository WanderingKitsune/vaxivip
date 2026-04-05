#ifndef VIDEO_INFO_HPP
#define VIDEO_INFO_HPP

#include "image_info.hpp"
#include <cstdint>

class VideoInfo {
public:
    ImageInfo image_info;
    uint32_t frame_total;
};

#endif
