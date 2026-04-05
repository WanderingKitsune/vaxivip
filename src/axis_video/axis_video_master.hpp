/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_video_master.hpp
 * @brief       AXI Stream Video Master (YUV frame buffer to AXI Stream)
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     Loads YUV planar frames and drives AXI4-Stream video data with
 *              configurable bits per channel and pixels per cycle.
 *
 * @ingroup axis_video
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2026/04/05  Initial release
 ******************************************************************************/

#ifndef AXIS_VIDEO_MASTER_HPP
#define AXIS_VIDEO_MASTER_HPP

#include "axis_master.hpp"
#include "frame_mem.hpp"
#include "log.hpp"
#include "axis_video_format.hpp"
#include <cstring>
#include <string>
#include <vector>

/**
 * @brief AXI4-Stream video master for YUV frame to stream conversion
 * @tparam BPC Bits per channel (default: 8)
 * @tparam PPC Pixels per cycle (default: 2)
 */
template <uint32_t BPC = 8, uint32_t PPC = 2>
class axis_video_master {
public:
    static constexpr uint32_t DATA_WIDTH = 3 * PPC * BPC;
    static constexpr uint32_t USER_WIDTH = 1;

    static_assert_bpc(BPC);
    static_assert_ppc(PPC);

    Log log;
    axis_master<DATA_WIDTH, 1, 1, USER_WIDTH> axis_mst;
    FrameInfo frame_info;
    bool end_of_frame = false;
    bool busy = false;
    bool done = false;

    /// @brief Constructor
    /// @param port AXI4-Stream master interface pointer
    axis_video_master(const axis_master_ptr<DATA_WIDTH, 1, 1, USER_WIDTH>& port) : axis_mst(port) {
        axis_mst.log.quiet = true;
    }

    /// @brief Destructor
    ~axis_video_master() { frames_.clear(); }

    /// @brief Load and start sending YUV frames over AXI4-Stream
    /// @param filename Path to YUV file to load
    /// @param info Frame information (width, height, color depth, etc.)
    /// @param start_frame Starting frame index (default: 0)
    /// @param frame_num Number of frames to send (0 = use info.frame_total)
    /// @return true if frames loaded and queued successfully, false otherwise
    bool send_frames(const std::string& filename,
                      const FrameInfo& info,
                      uint32_t start_frame = 0,
                      uint32_t frame_num = 0) {
        if (filename.empty()) {
            log.error("axis_video_master send_frames: empty filename");
            return false;
        }
        const uint32_t n = frame_num ? frame_num : info.frame_total;
        if (n == 0) {
            log.error("axis_video_master send_frames: frame_num is 0 and info.frame_total is 0");
            return false;
        }
        const uint64_t pix = static_cast<uint64_t>(info.width) * static_cast<uint64_t>(info.height);
        if (pix == 0) {
            log.error("axis_video_master send_frames: invalid frame size ", info.width, "x", info.height);
            return false;
        }

        FrameInfo fi = info;
        fi.frame_total = n;
        const uint32_t cd = fi.color_depth;
        if (cd == 0u || cd > 16u || cd > BPC) {
            log.error("axis_video_master send_frames: color_depth must be 1..16 and <= BPC");
            return false;
        }
        frame_info = fi;
        if (!frames_.init(fi)) {
            log.error("axis_video_master send_frames: FrameMem init failed");
            return false;
        }
        if (!frames_.read_file(filename, start_frame, n)) {
            log.error("axis_video_master send_frames: FrameMem read_file failed, file=", filename);
            return false;
        }
        log.info("[axis_video_master] send_frames ok: file=", filename,
                 ", size=", info.width, "x", info.height,
                 ", start_frame=", start_frame,
                 ", frame_num=", n);

        xfer_frame_total_ = n;
        busy = false;
        lines_sent_ = 0;
        done = false;
        end_of_frame = false;

        for (uint32_t f = 0; f < n; ++f)
            axis_pixel_pkg(f);

        busy = true;
        return true;
    }

    /// @brief Check if end-of-frame reached (sending completed)
    /// @return true if end-of-frame is reached, false otherwise
    bool eof() const { return end_of_frame; }
    /// @brief Update registered inputs from DUT
    void update_input() { axis_mst.update_input(); }

    /// @brief Drive outputs to DUT and check if sending complete
    void update_output() {
        if (!busy)
            return;
        end_of_frame = false;

        const bool tx_was_active = !axis_mst.tx_buf.empty();
        axis_mst.update_output();
        const bool tx_is_active = !axis_mst.tx_buf.empty();

        if (tx_was_active && !tx_is_active) {
            if (frame_info.height != 0) {
                ++lines_sent_;
                if ((lines_sent_ % frame_info.height) == 0) {
                    end_of_frame = true;
                    if (lines_sent_ / frame_info.height >= xfer_frame_total_)
                        done = true;
                }
            }
        }

        if (axis_mst.tx_buf.empty() && axis_mst.tx_queue.empty())
            busy = false;
    }

private:
    FrameMem frames_;
    uint32_t lines_sent_ = 0;
    uint32_t xfer_frame_total_ = 0;

    uint16_t sample_to_axis(uint16_t sample) const {
        const uint32_t cd = frame_info.color_depth;
        const uint32_t mask = (cd >= 32u) ? 0xFFFFu : ((1u << cd) - 1u);
        return static_cast<uint16_t>((static_cast<uint32_t>(sample) & mask) << (BPC - cd));
    }

    static void pack_beat(uint8_t* beat, const uint16_t* comp, uint32_t ncomp) {
        std::memset(beat, 0, DATA_WIDTH / 8u);
        for (uint32_t k = 0; k < ncomp; ++k) {
            const uint32_t bit_off = k * BPC;
            const uint32_t val = static_cast<uint32_t>(comp[k]) & ((1u << BPC) - 1u);
            for (uint32_t i = 0; i < BPC; ++i) {
                if ((val >> i) & 1u) {
                    const uint32_t b = bit_off + i;
                    beat[b / 8u] |= static_cast<uint8_t>(1u << (b % 8u));
                }
            }
        }
    }

    void axis_pixel_pkg(uint32_t frame_index) {
        constexpr uint32_t bytes_per_beat = DATA_WIDTH / 8u;
        constexpr uint32_t comp_per_beat = 3u * PPC;
        for (uint32_t y = 0; y < frame_info.height; ++y) {
            std::vector<uint16_t> ly, lu, lv;
            frames_.read_line(frame_index, 0, y, ly);
            frames_.read_line(frame_index, 1, y, lu);
            frames_.read_line(frame_index, 2, y, lv);
            const uint32_t nbeats = (frame_info.width + PPC - 1u) / PPC;
            std::vector<uint8_t> line_data;
            line_data.reserve(static_cast<size_t>(nbeats) * bytes_per_beat);
            std::vector<uint16_t> comp(comp_per_beat);
            for (uint32_t b = 0; b < nbeats; ++b) {
                for (uint32_t p = 0; p < PPC; ++p) {
                    const uint32_t x = b * PPC + p;
                    const uint16_t yv = x < frame_info.width ? ly[x] : 0;
                    const uint16_t uv = x < frame_info.width ? lu[x] : 0;
                    const uint16_t vv = x < frame_info.width ? lv[x] : 0;
                    comp[p * 3u + 0u] = sample_to_axis(yv);
                    comp[p * 3u + 1u] = sample_to_axis(uv);
                    comp[p * 3u + 2u] = sample_to_axis(vv);
                }
                uint8_t beat[sizeof(uint64_t) * 4]{};
                pack_beat(beat, comp.data(), comp_per_beat);
                for (uint32_t i = 0; i < bytes_per_beat; ++i)
                    line_data.push_back(beat[i]);
            }
            axis_mst.send(line_data, 0, 0, 0, y == 0);
        }
    }
};

#endif
