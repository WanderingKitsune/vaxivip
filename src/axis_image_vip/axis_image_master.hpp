/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_image_master.hpp
 * @brief       AXI Stream Image Master (BMP to AXI Stream)
 * @see         https://github.com/WanderingKitsune/vaxivip
 ******************************************************************************/

#ifndef AXIS_IMAGE_MASTER_HPP
#define AXIS_IMAGE_MASTER_HPP

#include "bmp.hpp"
#include "axis_vip.hpp"
#include <string>
#include <queue>

template <
    uint32_t BPC = 8,
    uint32_t PPC = 4
>
class axis_image_master {
public:
    /// @brief AXI4-Stream data width (RGB888, PPC pixels per beat)
    static constexpr uint32_t DATA_WIDTH = 3 * PPC * BPC;
    /// @brief AXI4-Stream TKEEP width
    static constexpr uint32_t TKEEP_WIDTH = DATA_WIDTH / 8;
    /// @brief AXI4-Stream USER width
    static constexpr uint32_t USER_WIDTH = 1;

    /// @brief Logger instance
    Log log;

    /// @brief Underlying AXI4-Stream master BFM
    axis_master<DATA_WIDTH, 1, 1, USER_WIDTH> axis_mst;

    /// @brief Loaded bitmap image
    Bitmap bmp;
    /// @brief Image width in pixels
    uint32_t img_width;
    /// @brief Image height in pixels
    uint32_t img_height;
    /// @brief Current pixel index
    uint32_t pixel_idx;
    /// @brief Whether the image is being sent
    bool sending;

    /**
     * @brief Constructor
     * @param port AXI4-Stream master interface pointer
     */
    axis_image_master(const axis_master_ptr<DATA_WIDTH, 1, 1, USER_WIDTH>& port)
        : axis_mst(port),
          img_width(0),
          img_height(0),
          pixel_idx(0),
          sending(false) {}

    /// @brief Load BMP file into internal buffer
    bool read_bmp(const std::string& filename) {
        bool success = bmp.read(filename);
        if (success) {
            img_width = bmp.width;
            img_height = bmp.height;
            log.info("[IMAGE-MST] Loaded BMP: ", filename,
                     " (", img_width, "x", img_height, ")");
        } else {
            log.error("[IMAGE-MST] Failed to load BMP: ", filename);
        }
        return success;
    }

    /// @brief Start sending the loaded image
    void send_image() {
        if (img_width == 0 || img_height == 0) {
            log.error("[IMAGE-MST] No image loaded");
            return;
        }
        sending   = true;
        pixel_idx = 0;
    }

    /// @brief One-shot helper: read BMP then start sending
    bool read_and_send_bmp(const std::string& filename) {
        if (!read_bmp(filename)) {
            return false;
        }
        send_image();
        return true;
    }

    /// @brief Check if image sending is in progress
    bool is_sending() const {
        return sending;
    }

    /// @brief Get image width in pixels
    uint32_t width() const {
        return img_width;
    }

    /// @brief Get image height in pixels
    uint32_t height() const {
        return img_height;
    }

    /// @brief Queue one AXI packet (thin wrapper to underlying AXIS master)
    void send(const std::vector<uint8_t>& data,
              uint32_t /*dest*/ = 0,
              uint32_t user = 0,
              bool sof = false,
              bool /*eol*/ = false) {
        // Map image parameters to generic AXIS fields:
        // id = 0, dest ignored here, user/sof passed through.
        axis_mst.send(data, 0, 0, user, sof);
    }

    /// @brief Update registered inputs from DUT
    void update_input() {
        axis_mst.update_input();
    }

    /// @brief Drive outputs to DUT
    void update_output() {
        axis_mst.update_output();
    }

    /// @brief Generate AXI packets from BMP pixels when ready
    void update() {
        if (!sending) {
            return;
        }
        if (!axis_mst.get_tready()) {
            return;
        }

        std::vector<uint8_t> pixel_data;

        // Pack pixels as byte-wise RGB: [R, G, B] per pixel
        bool start_of_frame = (pixel_idx == 0);
        for (uint32_t p = 0; p < PPC; p++) {
            uint32_t x = pixel_idx % img_width;
            uint32_t y = pixel_idx / img_width;

            if (y >= img_height) {
                sending = false;
                log.info("[IMAGE-MST] Image send complete. Resolution=",
                         img_width, "x", img_height);
                break;
            }

            uint32_t pixel = bmp.get_pixel(x, y);
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            pixel_data.push_back(r);
            pixel_data.push_back(g);
            pixel_data.push_back(b);

            pixel_idx++;
        }

        if (!pixel_data.empty()) {
            uint32_t last_x   = (pixel_idx - 1) % img_width;
            uint32_t last_y   = (pixel_idx - 1) / img_width;
            bool     eol      = (last_x >= img_width - PPC);

            // SOF only for very first beat of the whole frame
            bool sof = start_of_frame;
            axis_mst.send(pixel_data, 0, 0, 0, sof);

            if (eol) {
                log.info("[IMAGE-MST] Sent line ", last_y,
                         " / height=", img_height,
                         ", line_pixels_per_beat=", PPC);
            }
        }
    }
};

#endif
