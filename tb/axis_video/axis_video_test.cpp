/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_video_test.cpp
 * @brief       AXI4-Stream Video Testbench (C++)
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     Verilator C++ TB: video send/recv, save YUV. VIP: `src/axis_video/axis_video.hpp`
 *              (stream structs from `src/axis/` via that header; include dirs from `tb/axis_video/Makefile`).
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2026/04/05  Initial release
 ******************************************************************************/

#include "Vaxis_video_test.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "axis_video.hpp"
#include "axis_video_format.hpp"

#if !defined(TB_PIXFMT_YUV444) && !defined(TB_PIXFMT_YUV422) && !defined(TB_PIXFMT_YUV420)
#define TB_PIXFMT_YUV420
#endif

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    constexpr uint32_t BPC = 12;
    constexpr uint32_t PPC = 4;
    constexpr uint32_t DATA_WIDTH = 3 * PPC * BPC;

    Vaxis_video_test* top = new Vaxis_video_test;
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

    axis_video_master<BPC, PPC> video_mst{s_mst_port};
    axis_video_slave<BPC, PPC>  video_slv{m_slv_port};

    top->trace(tfp, 100);
    tfp->open("waveform.vcd");

    top->axis_clk = 0;
    top->axis_rst = 1;

    std::string input_yuv;
    std::string output_yuv = "out.yuv";
    std::remove(output_yuv.c_str());

    FrameInfo info;
#ifdef TB_PIXFMT_YUV422
    input_yuv = "test_320x240_yuv422p_2frame.yuv";
    info.pix_fmt = PIX_FMT_YUV422P;
#elif defined(TB_PIXFMT_YUV420)
    input_yuv = "test_320x240_yuv420p_2frame.yuv";
    info.pix_fmt = PIX_FMT_YUV420P;
#else
    input_yuv = "test_320x240_yuv444p_2frame.yuv";
    info.pix_fmt = PIX_FMT_YUV444P;
#endif
    info.width = 320;
    info.height = 240;
    info.color_depth = COLOR_DEPTH_8;
    info.frame_total = 1;

    std::cout << "Sending video: " << input_yuv << std::endl;

    uint64_t tick_count = 0;
    uint64_t clock_count = 0;
    const uint64_t max_ticks = 3 * uint64_t(info.width) * uint64_t(info.height) * uint64_t(info.frame_total) / uint64_t(PPC); // Arbitrary large number of ticks

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
                video_mst.send_frames(input_yuv, info, 0, 1);
                video_slv.recv_frames(output_yuv, info, false);
            }

            video_mst.update_input();
            video_slv.update_input();
        }

        top->eval();

        if (top->axis_clk) {
            video_mst.update_output();
            video_slv.update_output();

            if (started && video_mst.done && video_slv.done) break;
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
