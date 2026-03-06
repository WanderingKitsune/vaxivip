/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_image_slave.hpp
 * @brief       AXI Stream Image Slave (AXI Stream to BMP)
 * @see         https://github.com/WanderingKitsune/vaxivip
 ******************************************************************************/

#ifndef AXIS_IMAGE_SLAVE_HPP
#define AXIS_IMAGE_SLAVE_HPP

#include "bmp.hpp"
#include "axis_vip.hpp"
#include <string>
#include <queue>

template <
    uint32_t BPC = 8,
    uint32_t PPC = 4
>
class axis_image_slave {
public:
    /// @brief AXI4-Stream data width (RGB888, PPC pixels per beat)
    static constexpr uint32_t DATA_WIDTH = 3 * PPC * BPC;
    /// @brief AXI4-Stream TKEEP width
    static constexpr uint32_t TKEEP_WIDTH = DATA_WIDTH / 8;
    /// @brief AXI4-Stream USER width
    static constexpr uint32_t USER_WIDTH = 1;

    /// @brief Logger instance
    Log log;

    /// @brief Underlying AXI4-Stream slave BFM
    axis_slave<DATA_WIDTH, 1, 1, USER_WIDTH> axis_slv;

    /// @brief Receive bitmap
    Bitmap bmp;
    /// @brief Image width in pixels
    uint32_t img_width;
    /// @brief Image height in pixels
    uint32_t img_height;
    /// @brief Current pixel index
    uint32_t pixel_idx;
    /// @brief Receiving state
    bool receiving;

    /**
     * @brief Constructor
     * @param port AXI4-Stream slave interface pointer
     */
    axis_image_slave(const axis_slave_ptr<DATA_WIDTH, 1, 1, USER_WIDTH>& port)
        : axis_slv(port),
          img_width(0),
          img_height(0),
          pixel_idx(0),
          receiving(false) {}

    /// @brief Prepare to receive an image
    void receive_image(uint32_t width, uint32_t height) {
        img_width  = width;
        img_height = height;
        pixel_idx  = 0;
        receiving  = true;
        bmp.create(width, height);
        log.info("[IMAGE-SLV] Ready to receive image: ",
                 width, "x", height);
    }

    /// @brief Check if still receiving
    bool is_receiving() const {
        return receiving;
    }

    /// @brief Check if internal packet queue is empty
    bool empty() {
        return axis_slv.empty();
    }

    /// @brief Receive a packet from internal queue
    ssize_t recv(std::vector<uint8_t>& dst_buf) {
        return axis_slv.recv(dst_buf);
    }

    /// @brief Set TREADY state
    void set_tready(bool ready) {
        axis_slv.set_tready(ready);
    }

    /// @brief Update registered inputs from DUT
    void update_input() {
        axis_slv.update_input();
    }

    /// @brief Drive outputs to DUT and push packets into internal queue
    void update_output() {
        axis_slv.update_output();
    }

    /// @brief Consume received AXI packets and write them into BMP
    void update() {
        if (!receiving) {
            return;
        }

        std::vector<uint8_t> data;
        ssize_t size = recv(data);

        if (size > 0) {
            // Expect 3 bytes per pixel: [R, G, B]
            uint32_t bytes_per_pixel = 3 * ((BPC + 7) / 8);
            uint32_t pixels_in_batch = size / bytes_per_pixel;

            for (uint32_t p = 0; p < pixels_in_batch; p++) {
                uint32_t x = pixel_idx % img_width;
                uint32_t y = pixel_idx / img_width;

                if (y >= img_height) {
                    receiving = false;
                    log.info("[IMAGE-SLV] Image receive complete. Resolution=",
                             img_width, "x", img_height);
                    return;
                }

                uint32_t offset = p * bytes_per_pixel;
                uint8_t  r      = data[offset + 0];
                uint8_t  g      = data[offset + 1];
                uint8_t  b      = data[offset + 2];

                uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
                bmp.set_pixel(x, y, color);

                pixel_idx++;
            }
        }
    }

    bool save_bmp(const std::string& filename) {
        bool success = bmp.write(filename);
        if (success) {
            log.info("[IMAGE-SLV] Saved BMP: ", filename);
        } else {
            log.error("[IMAGE-SLV] Failed to save BMP: ", filename);
        }
        return success;
    }
};

#endif
