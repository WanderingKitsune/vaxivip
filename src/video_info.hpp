#ifndef VIDEO_INFO_HPP
#define VIDEO_INFO_HPP

#include "image_info.hpp"

class VideoInfo : public ImageInfo {
public:
    uint32_t frame_total = 0;
};

#endif
