/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        frame_mem.cpp
 * @brief       Frame memory manager implementation for YUV planar video
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     Implementation of FrameInfo and FrameMem classes for YUV planar
 *              video frame storage and I/O operations.
 *
 * @ingroup axis_video
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2026/04/05   Initial release
 ******************************************************************************/

#include "frame_mem.hpp"

#include <climits>
#include <cstdio>
#include <cstring>

// Destructor - clears frame data
FrameMem::~FrameMem() {
    clear();
}

// Clear all frame data
void FrameMem::clear() {
    for (auto& pl : planes_)
        pl.clear();
    info_ = FrameInfo{};
}

// Get plane width for given plane index
uint32_t FrameInfo::plane_width(uint32_t plane_index) const {
    if (plane_index > 2)
        return 0;
    switch (pix_fmt) {
        case PIX_FMT_YUV444P:
            return width;
        default:
            return 0;
    }
}

// Get plane height for given plane index
uint32_t FrameInfo::plane_height(uint32_t plane_index) const {
    if (plane_index > 2)
        return 0;
    switch (pix_fmt) {
        case PIX_FMT_YUV444P:
            return height;
        default:
            return 0;
    }
}

// Get total number of planes
uint32_t FrameInfo::plane_total() const {
    uint32_t t = 0;
    for (uint32_t p = 0; p < 3; ++p)
        t += static_cast<uint32_t>(plane_samples(p));
    return t;
}

// Check if plane index is valid
bool FrameInfo::plane_ok(uint32_t plane_index) const {
    return plane_width(plane_index) > 0 && plane_height(plane_index) > 0;
}

// Get total number of samples in plane
std::size_t FrameInfo::plane_samples(uint32_t plane_index) const {
    return static_cast<std::size_t>(plane_width(plane_index)) * static_cast<std::size_t>(plane_height(plane_index));
}

// Calculate memory offset for pixel access
std::size_t FrameMem::plane_offset(uint32_t frame_index, uint32_t plane_index, uint32_t y, uint32_t x) const {
    const std::size_t stride = info_.plane_samples(plane_index);
    return static_cast<std::size_t>(frame_index) * stride + static_cast<std::size_t>(y) * info_.plane_width(plane_index) +
           x;
}

// Initialize frame memory with given frame information
bool FrameMem::init(const FrameInfo& info) {
    if (info.frame_total == 0 || info.width == 0 || info.height == 0 || info.pix_fmt == PIX_FMT_NONE)
        return false;
    info_ = info;
    for (uint32_t p = 0; p < 3; ++p) {
        if (!info_.plane_ok(p)) {
            for (auto& pl : planes_)
                pl.clear();
            return false;
        }
    }
    for (uint32_t p = 0; p < 3; ++p)
        planes_[p].resize(static_cast<size_t>(info_.frame_total) * info_.plane_samples(p));
    return true;
}

// Read YUV planar frames from file
bool FrameMem::read_file(const std::string& file_path, uint32_t start_frame, uint32_t frame_num) {
    const uint32_t n = frame_num ? frame_num : info_.frame_total;
    if (n == 0 || n > info_.frame_total)
        return false;
    FILE* fp = std::fopen(file_path.c_str(), "rb");
    if (!fp)
        return false;
    const bool planar8 = info_.color_depth == static_cast<uint32_t>(COLOR_DEPTH_8);
    const uint64_t frame_bytes =
        planar8 ? static_cast<uint64_t>(info_.plane_total())
                : static_cast<uint64_t>(info_.plane_total()) * sizeof(uint16_t);
    const uint64_t skip = static_cast<uint64_t>(start_frame) * frame_bytes;
    if (skip > static_cast<uint64_t>(LONG_MAX)) {
        std::fclose(fp);
        return false;
    }
    if (std::fseek(fp, static_cast<long>(skip), SEEK_SET) != 0) {
        std::fclose(fp);
        return false;
    }
    if (planar8) {
        for (uint32_t f = 0; f < n; ++f) {
            for (uint32_t p = 0; p < 3; ++p) {
                const size_t ps = info_.plane_samples(p);
                std::vector<uint8_t> tmp(ps);
                if (std::fread(tmp.data(), 1, ps, fp) != ps) {
                    std::fclose(fp);
                    return false;
                }
                std::vector<uint16_t>& pv = planes_[p];
                const size_t base = static_cast<size_t>(f) * ps;
                for (size_t k = 0; k < ps; ++k)
                    pv[base + k] = static_cast<uint16_t>(tmp[k]);
            }
        }
        std::fclose(fp);
        return true;
    }
    size_t total = 0;
    for (uint32_t p = 0; p < 3; ++p)
        total += static_cast<size_t>(n) * info_.plane_samples(p);
    const size_t need = total * sizeof(uint16_t);
    std::vector<uint8_t> buf(need);
    const size_t got = std::fread(buf.data(), 1, need, fp);
    std::fclose(fp);
    if (got != need)
        return false;
    auto rd = [&buf](size_t& off) -> uint16_t {
        uint16_t v;
        std::memcpy(&v, buf.data() + off, sizeof(v));
        off += sizeof(v);
        return v;
    };
    size_t off = 0;
    for (uint32_t f = 0; f < n; ++f) {
        for (uint32_t p = 0; p < 3; ++p) {
            std::vector<uint16_t>& pv = planes_[p];
            const size_t ps = info_.plane_samples(p);
            const size_t base = static_cast<size_t>(f) * ps;
            for (size_t k = 0; k < ps; ++k)
                pv[base + k] = rd(off);
        }
    }
    return true;
}

// Write YUV planar frames to file
bool FrameMem::write_file(const std::string& file_path, bool append) const {
    const bool planar8 = info_.color_depth == static_cast<uint32_t>(COLOR_DEPTH_8);
    const char* mode = append ? "ab" : "wb";
    FILE* fp = std::fopen(file_path.c_str(), mode);
    if (!fp)
        return false;
    if (planar8) {
        std::vector<uint8_t> tmp;
        for (uint32_t f = 0; f < info_.frame_total; ++f) {
            for (uint32_t p = 0; p < 3; ++p) {
                const std::vector<uint16_t>& pv = planes_[p];
                const size_t ps = info_.plane_samples(p);
                const size_t base = static_cast<size_t>(f) * ps;
                tmp.resize(ps);
                for (size_t k = 0; k < ps; ++k)
                    tmp[k] = static_cast<uint8_t>(pv[base + k] & 0xFFu);
                if (std::fwrite(tmp.data(), 1, ps, fp) != ps) {
                    std::fclose(fp);
                    return false;
                }
            }
        }
        std::fclose(fp);
        return true;
    }
    for (uint32_t f = 0; f < info_.frame_total; ++f) {
        for (uint32_t p = 0; p < 3; ++p) {
            const std::vector<uint16_t>& pv = planes_[p];
            const size_t ps = info_.plane_samples(p);
            const size_t base = static_cast<size_t>(f) * ps;
            if (std::fwrite(pv.data() + base, sizeof(uint16_t), ps, fp) != ps) {
                std::fclose(fp);
                return false;
            }
        }
    }
    std::fclose(fp);
    return true;
}

// Read a line of samples from frame memory
bool FrameMem::read_line(uint32_t frame_index, uint32_t plane_index, uint32_t y, std::vector<uint16_t>& line) const {
    if (plane_index > 2 || frame_index >= info_.frame_total || y >= info_.plane_height(plane_index))
        return false;
    const std::vector<uint16_t>& pv = planes_[plane_index];
    const uint32_t pw = info_.plane_width(plane_index);
    line.resize(pw);
    const size_t ro = plane_offset(frame_index, plane_index, y, 0);
    for (uint32_t x = 0; x < pw; ++x)
        line[x] = pv[ro + x];
    return true;
}

// Write a line of samples to frame memory
bool FrameMem::write_line(uint32_t frame_index, uint32_t plane_index, uint32_t y, const std::vector<uint16_t>& data) {
    if (plane_index > 2 || frame_index >= info_.frame_total || y >= info_.plane_height(plane_index))
        return false;
    const uint32_t pw = info_.plane_width(plane_index);
    if (data.size() < pw)
        return false;
    std::vector<uint16_t>& pv = planes_[plane_index];
    const size_t ro = plane_offset(frame_index, plane_index, y, 0);
    for (uint32_t x = 0; x < pw; ++x)
        pv[ro + x] = data[x];
    return true;
}

// Read a single pixel sample from frame memory
uint16_t FrameMem::read_pixel(uint32_t frame_index, uint32_t plane_index, uint32_t x, uint32_t y) const {
    if (plane_index > 2 || frame_index >= info_.frame_total || x >= info_.plane_width(plane_index) ||
        y >= info_.plane_height(plane_index))
        return 0;
    const std::vector<uint16_t>& pv = planes_[plane_index];
    return pv[plane_offset(frame_index, plane_index, y, x)];
}

// Write a single pixel sample to frame memory
bool FrameMem::write_pixel(uint32_t frame_index, uint32_t plane_index, uint32_t x, uint32_t y, uint32_t value) {
    if (plane_index > 2 || frame_index >= info_.frame_total || x >= info_.plane_width(plane_index) ||
        y >= info_.plane_height(plane_index))
        return false;
    std::vector<uint16_t>& pv = planes_[plane_index];
    pv[plane_offset(frame_index, plane_index, y, x)] = static_cast<uint16_t>(value & 0xFFFFu);
    return true;
}
