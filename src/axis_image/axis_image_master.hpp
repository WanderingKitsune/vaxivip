/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_image_master.hpp
 * @brief       AXI Stream Image Master (BMP to AXI Stream)
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     Loads BMP and drives AXI4-Stream image data with configurable
 *              bits per channel and pixels per cycle.
 *
 * @ingroup axis_image
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/30  Initial release
 ******************************************************************************/

#ifndef AXIS_IMAGE_MASTER_HPP
#define AXIS_IMAGE_MASTER_HPP

#include "axis_master.hpp"
#include "bmp.hpp"
#include "log.hpp"
#include <string>
#include <queue>

/**
 * @brief AXI4-Stream image master for BMP to stream conversion
 * @tparam BPC Bits per channel (default: 8)
 * @tparam PPC Pixels per cycle (default: 4)
 */
template <
    uint32_t BPC = 8,
    uint32_t PPC = 4
>
class axis_image_master {
public:
    /// @brief AXI4-Stream data width in bits (RGB888, PPC pixels per beat)
    static constexpr uint32_t DATA_WIDTH = 3 * PPC * BPC;
    /// @brief AXI4-Stream TKEEP width in bytes
    static constexpr uint32_t TKEEP_WIDTH = DATA_WIDTH / 8;
    /// @brief AXI4-Stream USER width in bits (1 bit for SOF)
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
          sending(false) {
        axis_mst.log.quiet = true;
    }

    /// @brief Load BMP file into internal buffer
    /// @param filename Path to BMP file to load
    /// @return true if BMP loaded successfully, false otherwise
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

    /// @brief Start sending the loaded image over AXI4-Stream
    void send_image() {
        if (img_width == 0 || img_height == 0) {
            log.error("[IMAGE-MST] No image loaded");
            return;
        }
        sending   = true;
        pixel_idx = 0;
    }

    /// @brief One-shot helper: read BMP then start sending
    /// @param filename Path to BMP file to load and send
    /// @return true if BMP loaded successfully and sending started, false otherwise
    bool read_and_send_bmp(const std::string& filename) {
        if (!read_bmp(filename)) {
            return false;
        }
        send_image();
        return true;
    }

    /// @brief Load BMP and enqueue entire frame for streaming
    /// @param filename Path to BMP file to load and send
    /// @return true if BMP loaded successfully and frame enqueued, false otherwise
    bool send_frame(const std::string& filename) {
        if (!read_bmp(filename)) {
            return false;
        }
        enqueue_frame_lines();
        sending = true;
        return true;
    }

    /// @brief Check if end-of-frame reached (sending completed)
    /// @return true if sending is complete, false if still sending
    bool eof() const {
        return !sending;
    }

    /// @brief Check if image sending is in progress
    /// @return true if image is currently being sent, false otherwise
    bool is_sending() const {
        return sending;
    }

    /// @brief Get image width in pixels
    /// @return Current image width in pixels (0 if no image loaded)
    uint32_t width() const {
        return img_width;
    }

    /// @brief Get image height in pixels
    /// @return Current image height in pixels (0 if no image loaded)
    uint32_t height() const {
        return img_height;
    }

    /// @brief Update registered inputs from DUT
    void update_input() {
        axis_mst.update_input();
    }

    /// @brief Drive outputs to DUT and check if sending complete
    void update_output() {
        axis_mst.update_output();
        if (!sending) return;
        if (axis_mst.tx_buf.empty() && axis_mst.tx_queue.empty()) {
            sending = false;
            log.info("[IMAGE-MST] Image send complete. Resolution=",
                     img_width, "x", img_height);
        }
    }

private:
    /// @brief Enqueue all image lines as AXI4-Stream transactions
    void enqueue_frame_lines() {
        if (img_width == 0 || img_height == 0) return;

        for (uint32_t y = 0; y < img_height; y++) {
            std::vector<uint8_t> line_data;
            line_data.reserve(img_width * 3);

            for (uint32_t x = 0; x < img_width; x++) {
                uint32_t pixel = bmp.get_pixel(x, y);
                uint8_t  r     = (pixel >> 16) & 0xFF;
                uint8_t  g     = (pixel >> 8) & 0xFF;
                uint8_t  b     = pixel & 0xFF;
                line_data.push_back(r);
                line_data.push_back(g);
                line_data.push_back(b);
            }

            bool sof = (y == 0);
            axis_mst.send(line_data, 0, 0, 0, sof);
        }
    }
};

#endif
