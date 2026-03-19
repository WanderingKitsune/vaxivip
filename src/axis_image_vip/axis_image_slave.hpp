/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_image_slave.hpp
 * @brief       AXI Stream Image Slave (AXI Stream to BMP)
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     Receives AXI4-Stream image by preset resolution and writes BMP (recv_frame / update_output).
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/30  Initial release
 ******************************************************************************/

#ifndef AXIS_IMAGE_SLAVE_HPP
#define AXIS_IMAGE_SLAVE_HPP

#include "axis_image_bfm.hpp"
#include "bmp.hpp"
#include "log.hpp"
#include <string>
#include <queue>

template <
    uint32_t BPC = 8,
    uint32_t PPC = 4
>
class axis_image_slave {
public:
    static constexpr uint32_t DATA_WIDTH = 3 * PPC * BPC;
    static constexpr uint32_t TKEEP_WIDTH = DATA_WIDTH / 8;
    static constexpr uint32_t USER_WIDTH = 1;

    Log log;
    axis_image_bfm_slv<DATA_WIDTH, 1, 1, USER_WIDTH> axis_slv;
    Bitmap bmp;
    uint32_t img_width;
    uint32_t img_height;
    uint32_t pixel_idx;
    bool receiving;

    axis_image_slave(const axis_slave_ptr<DATA_WIDTH, 1, 1, USER_WIDTH>& port)
        : axis_slv(port),
          img_width(0),
          img_height(0),
          pixel_idx(0),
          receiving(false) {}

    void receive_image(uint32_t width, uint32_t height) {
        img_width  = width;
        img_height = height;
        pixel_idx  = 0;
        receiving  = true;
        bmp.create(width, height);
        log.info("[IMAGE-SLV] Ready to receive image: ",
                 width, "x", height);
    }

    void recv_frame(uint32_t width, uint32_t height) {
        receive_image(width, height);
    }

    bool is_receiving() const {
        return receiving;
    }

    bool eof() const {
        return !receiving;
    }

    bool empty() {
        return axis_slv.empty();
    }

    void set_tready(bool ready) {
        axis_slv.set_tready(ready);
    }

    void update_input() {
        axis_slv.update_input();
    }

    void update_output() {
        axis_slv.update_output();
        if (!receiving) return;

        const uint32_t total_pixels = img_width * img_height;
        std::vector<uint8_t> data;
        ssize_t size = axis_slv.recv(data);
        if (size <= 0) return;

        uint32_t bpp = 3 * ((BPC + 7) / 8);
        if (bpp == 0) bpp = 3;

        const uint32_t sz = static_cast<uint32_t>(size);
        bool pkt_ok = true;

        if (sz % bpp != 0) {
            log.error("[IMAGE-SLV] packet size not multiple of pixel stride: size=", sz, " bpp=", bpp);
            pkt_ok = false;
        }

        const uint32_t line_b = img_width * bpp;
        if (line_b == 0) {
            receiving = false;
            return;
        }

        if ((pixel_idx % img_width) != 0) {
            log.error("[IMAGE-SLV] packet not at row boundary (preset W=", img_width,
                      " pixel_idx=", pixel_idx, ")");
            pkt_ok = false;
        } else {
            const uint32_t rows_left = (total_pixels - pixel_idx) / img_width;
            if (sz % line_b != 0) {
                log.error("[IMAGE-SLV] line width mismatch preset: bytes=", sz,
                          " expect multiple of ", line_b, " (W=", img_width, " bpp=", bpp, ")");
                pkt_ok = false;
            } else {
                const uint32_t nlines = sz / line_b;
                if (nlines > rows_left) {
                    log.error("[IMAGE-SLV] row count exceeds preset: packet_lines=", nlines,
                              " rows_left=", rows_left, " (preset H=", img_height, ")");
                    pkt_ok = false;
                }
            }
        }

        if (!pkt_ok) {
            log.error("[IMAGE-SLV] frame recv aborted (preset ", img_width, "x", img_height, ")");
            receiving = false;
            return;
        }

        const uint32_t p0 = pixel_idx;
        for (uint32_t off = 0; off + bpp - 1 < static_cast<uint32_t>(size) && pixel_idx < total_pixels;
             off += bpp) {
            uint32_t x = pixel_idx % img_width;
            uint32_t y = pixel_idx / img_width;
            uint8_t  r = data[off + 0];
            uint8_t  g = data[off + 1];
            uint8_t  b = data[off + 2];
            uint32_t color = (0xFFu << 24) | (static_cast<uint32_t>(r) << 16) |
                             (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
            bmp.set_pixel(x, y, color);
            pixel_idx++;
        }

        const uint32_t consumed_b = (pixel_idx - p0) * bpp;
        if (consumed_b < static_cast<uint32_t>(size)) {
            log.error("[IMAGE-SLV] extra data vs preset ", img_width, "x", img_height, ": ",
                      static_cast<uint32_t>(size) - consumed_b, " bytes");
        }

        if (pixel_idx >= total_pixels) {
            receiving = false;
            log.info("[IMAGE-SLV] Image receive complete. Resolution=",
                     img_width, "x", img_height);
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

    bool save_frame(const std::string& filename) {
        return save_bmp(filename);
    }
};

#endif
