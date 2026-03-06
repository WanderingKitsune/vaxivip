/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_image_test.cpp
 * @brief       AXI4-Stream Image Testbench (C++)
 * @see         https://github.com/WanderingKitsune/vaxivip
 ******************************************************************************/

#include "Vaxis_image_test.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "axis_image_vip.hpp"

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    constexpr uint32_t BPC        = 8;
    constexpr uint32_t PPC        = 4;
    constexpr uint32_t DATA_WIDTH = 3 * PPC * BPC;

    Vaxis_image_test* top = new Vaxis_image_test;
    VerilatedVcdC*    tfp = new VerilatedVcdC;

    axis_ptr<DATA_WIDTH, 1, 1, 1> s_axis_ptr;
    axis_ptr<DATA_WIDTH, 1, 1, 1> m_axis_ptr;

    s_axis_ptr.tdata  = &(top->s_tdata);
    s_axis_ptr.tkeep  = &(top->s_tkeep);
    s_axis_ptr.tstrb  = &(top->s_tstrb);
    s_axis_ptr.tid    = &(top->s_tid);
    s_axis_ptr.tdest  = &(top->s_tdest);
    s_axis_ptr.tuser  = &(top->s_tuser);
    s_axis_ptr.tlast  = &(top->s_tlast);
    s_axis_ptr.tvalid = &(top->s_tvalid);
    s_axis_ptr.tready = &(top->s_tready);

    m_axis_ptr.tdata  = &(top->m_tdata);
    m_axis_ptr.tkeep  = &(top->m_tkeep);
    m_axis_ptr.tstrb  = &(top->m_tstrb);
    m_axis_ptr.tid    = &(top->m_tid);
    m_axis_ptr.tdest  = &(top->m_tdest);
    m_axis_ptr.tuser  = &(top->m_tuser);
    m_axis_ptr.tlast  = &(top->m_tlast);
    m_axis_ptr.tvalid = &(top->m_tvalid);
    m_axis_ptr.tready = &(top->m_tready);

    axis_master_ptr<DATA_WIDTH, 1, 1, 1> s_mst_port(s_axis_ptr);
    axis_slave_ptr<DATA_WIDTH, 1, 1, 1> m_slv_port(m_axis_ptr);

    axis_image_master<BPC, PPC> img_mst{s_mst_port};
    axis_image_slave<BPC, PPC>  img_slv{m_slv_port};

    top->trace(tfp, 100);
    tfp->open("waveform.vcd");

    top->axis_clk = 0;
    top->axis_rst = 1;

    std::string input_bmp = "test_images/in.bmp";
    std::string output_bmp = "test_images/out.bmp";

    Bitmap bmp;
    if (!bmp.read(input_bmp)) {
        std::cerr << "Failed to read input BMP: " << input_bmp << std::endl;
        return -1;
    }

    std::cout << "Loaded image: " << bmp.width << "x" << bmp.height << std::endl;

    uint64_t tick_count = 0;
    uint64_t clock_count = 0;
    const uint64_t max_ticks = 500000;

    bool image_sent = false;
    bool image_received = false;
    uint32_t pixel_idx = 0;
    uint32_t recv_pixels = 0;
    uint32_t expected_pixels = bmp.width * bmp.height;

    Bitmap recv_bmp;
    recv_bmp.create(bmp.width, bmp.height);

    while (!Verilated::gotFinish() && tick_count < max_ticks) {
        top->axis_clk = !top->axis_clk;
        tick_count++;

        if (tick_count == 10) {
            top->axis_rst = 0;
        }

        if (top->axis_clk) {
            clock_count++;

            if (!image_sent && clock_count > 20) {
                std::vector<uint8_t> pixel_data;

                bool is_first_packet = (pixel_idx == 0);
                uint32_t start_pixel_idx = pixel_idx;

                for (uint32_t p = 0; p < PPC; p++) {
                    uint32_t x = pixel_idx % bmp.width;
                    uint32_t y = pixel_idx / bmp.width;

                    if (y >= bmp.height) {
                        image_sent = true;
                        break;
                    }

                    uint32_t pixel = bmp.get_pixel(x, y);
                    uint8_t r = (pixel >> 16) & 0xFF;
                    uint8_t g = (pixel >> 8) & 0xFF;
                    uint8_t b = pixel & 0xFF;

                    // Pack as byte-wise RGB
                    pixel_data.push_back(r);
                    pixel_data.push_back(g);
                    pixel_data.push_back(b);

                    pixel_idx++;

                    if (pixel_idx >= bmp.width * bmp.height) {
                        image_sent = true;
                        break;
                    }
                }

                if (!pixel_data.empty()) {
                    uint32_t remaining = (start_pixel_idx + PPC > bmp.width * bmp.height) ? 
                                        (bmp.width * bmp.height - start_pixel_idx) : 
                                        (bmp.width - (start_pixel_idx % bmp.width));
                    bool eol = (remaining <= PPC);

                    std::cout << "pixel_idx=" << pixel_idx << " start_pixel_idx=" << start_pixel_idx << " remaining=" << remaining << " eol=" << eol << std::endl;

                    img_mst.send(pixel_data, 0, is_first_packet ? 1 : 0, is_first_packet, eol);

                    if (eol) {
                        std::cout << "Sent line: " << ((pixel_idx - 1) / bmp.width) << std::endl;
                    }
                }
            }

            // Follow Verilator TB convention: update_input -> eval -> update_output on posedge
            img_mst.update_input();
            img_slv.update_input();
        }

        top->eval();

        if (top->axis_clk) {
            img_mst.update_output();
            img_slv.update_output();

            std::vector<uint8_t> recv_data;
            ssize_t size = img_slv.recv(recv_data);

            if (size > 0 && !image_received) {
                uint32_t bytes_per_pixel = 3 * ((BPC + 7) / 8);
                uint32_t pixels_in_batch = size / bytes_per_pixel;

                for (uint32_t p = 0; p < pixels_in_batch; p++) {
                    uint32_t x = recv_pixels % recv_bmp.width;
                    uint32_t y = recv_pixels / recv_bmp.width;

                    if (y >= recv_bmp.height) {
                        image_received = true;
                        break;
                    }

                    uint32_t offset = p * bytes_per_pixel;
                    uint8_t r = recv_data[offset + 0];
                    uint8_t g = recv_data[offset + 1];
                    uint8_t b = recv_data[offset + 2];

                    uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
                    recv_bmp.set_pixel(x, y, color);

                    recv_pixels++;
                }

                if (recv_pixels >= expected_pixels) {
                    image_received = true;
                    std::cout << "Received all pixels: " << recv_pixels << std::endl;
                }
            }

            if (image_sent && image_received) {
                break;
            }
        }

        top->eval();
        tfp->dump(tick_count);
    }

    tfp->close();

    if (recv_bmp.write(output_bmp)) {
        std::cout << "Saved output BMP: " << output_bmp << std::endl;
    } else {
        std::cerr << "Failed to save output BMP" << std::endl;
    }

    delete tfp;
    delete top;

    std::cout << "Simulation finished at cycle " << clock_count << std::endl;
    return 0;
}
