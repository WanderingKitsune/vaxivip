/******************************************************************************
 * Copyright (C) 2025 dozecat. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_image_test.cpp
 * @brief       AXI4-Stream Image Testbench (C++)
 * @see         https://github.com/dozecat/vaxivip
 *
 * @details     Verilator C++ TB: one image send/recv, save BMP. VIP: `src/axis_image/axis_image_vip.hpp`
 *              (stream structs from `src/axis/` via that header; include dirs from `tb/axis_image/Makefile`).
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/30  Initial release
 ******************************************************************************/

#include "Vaxis_image_test.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "axis_image.hpp"
#include "axis_video_format.hpp"

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

    std::string input_bmp = "in.bmp";
    std::string output_bmp = "out.bmp";

    ImageInfo info;

    std::cout << "Sending image: " << input_bmp << std::endl;

    uint64_t tick_count = 0;
    uint64_t clock_count = 0;
    const uint64_t max_ticks = 500000;

    bool started = false;

    while (!Verilated::gotFinish() && tick_count < max_ticks) {
        top->axis_clk = !top->axis_clk;
        tick_count++;

        if (tick_count == 10) {
            top->axis_rst = 0;
        }

        if (top->axis_clk) {
            clock_count++;
            if (!started && clock_count > 20) {
                started = true;
                img_mst.send_frame(input_bmp, &info);
                img_slv.recv_frame(&info, output_bmp);
            }
            img_mst.update_input();
            img_slv.update_input();
        }

        top->eval();

        if (top->axis_clk) {
            img_mst.update_output();
            img_slv.update_output();

            if (started && img_mst.eof() && img_slv.eof()) break;
        }

        top->eval();
        tfp->dump(tick_count);
    }

    tfp->close();

    delete tfp;
    delete top;

    std::cout << "Simulation finished at cycle " << clock_count << std::endl;
    return 0;
}
