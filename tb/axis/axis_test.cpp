/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_test.cpp
 * @brief       AXI4-Stream Testbench (C++)
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     This file contains the C++ testbench for verifying the
 *              AXI4-Stream interface using Verilator.
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/25  Initial release
 ******************************************************************************/

#include "Vaxis_test.h"
// #include "Vaxis_test__Dpi.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <svdpi.h>
#include <iostream>
#include "axis.hpp"
#include "axis_vip.hpp"

/**
 * @brief Connect BFM pointers to Verilator model signals
 * @param s_axis_ptr Slave interface pointer
 * @param m_axis_ptr Master interface pointer
 * @param top        Pointer to the Verilated top module
 */
void axis_connect(axis_ptr<256, 8, 1, 1>& s_axis_ptr, axis_ptr<256, 8, 1, 1>& m_axis_ptr, Vaxis_test* top) {
    s_axis_ptr.tdata  = &(top->s_tdata);
    s_axis_ptr.tkeep  = &(top->s_tkeep);
    s_axis_ptr.tdest  = &(top->s_tdest);
    s_axis_ptr.tstrb  = &(top->s_tstrb);
    s_axis_ptr.tid    = &(top->s_tid);
    s_axis_ptr.tuser  = &(top->s_tuser);
    s_axis_ptr.tlast  = &(top->s_tlast);
    s_axis_ptr.tvalid = &(top->s_tvalid);
    s_axis_ptr.tready = &(top->s_tready);

    m_axis_ptr.tdata  = &(top->m_tdata);
    m_axis_ptr.tkeep  = &(top->m_tkeep);
    m_axis_ptr.tdest  = &(top->m_tdest);
    m_axis_ptr.tstrb  = &(top->m_tstrb);
    m_axis_ptr.tid    = &(top->m_tid);
    m_axis_ptr.tuser  = &(top->m_tuser);
    m_axis_ptr.tlast  = &(top->m_tlast);
    m_axis_ptr.tvalid = &(top->m_tvalid);
    m_axis_ptr.tready = &(top->m_tready);
}

/**
 * @brief Main testbench function
 */
int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    Vaxis_test* top = new Vaxis_test;
    VerilatedVcdC* tfp = new VerilatedVcdC;

    axis_ptr<256, 8, 1, 1> axis_in_ptr;
    axis_ptr<256, 8, 1, 1> axis_out_ptr;

    axis_connect(axis_in_ptr, axis_out_ptr, top);
    if (!axis_in_ptr.check()) {
        std::cerr << "axis_in_ptr connection failed!" << std::endl;
        return -1;
    }
    if (!axis_out_ptr.check()) {
        std::cerr << "axis_out_ptr connection failed!" << std::endl;
        return -1;
    }

    axis_master<256> axis_mst(axis_in_ptr);
    axis_slave<256> axis_slv(axis_out_ptr);

    top->trace(tfp, 100);
    tfp->open("waveform.vcd");

    top->axis_clk = 0;
    top->axis_rst = 1;

    uint64_t tick_count = 0;
    uint64_t clock_count = 0;
    const uint64_t max_ticks = 100;

    while (!Verilated::gotFinish() && tick_count < max_ticks) {
        // clock toggle
        top->axis_clk = !top->axis_clk;
        tick_count++;
        // reset release after 5 ticks
        if (tick_count == 5) {
            top->axis_rst = 0;
        }
        // Sample signals before posedge
        if (top->axis_clk) {
            axis_mst.update_input();
            axis_slv.update_input();
        }
        top->eval();
        // Update signals at posedge
        if (top->axis_clk) {
            clock_count++;
            if (clock_count == 5) {
                char data_num[100];
                for (int i = 0; i < 100; i++) {
                    data_num[i] = i;
                }
                axis_mst.send(data_num, 77);
            }
            if (clock_count == 40) {
                char data_recv[100];
                std::cout << "Data at cycle 40: ";
                ssize_t recv_size = axis_slv.recv(100, data_recv);
                for (int i = 0; i < recv_size; i++) {
                    std::cout << (int)data_recv[i] << " ";
                }
                std::cout << "Size: " << recv_size << std::endl;
            }
            axis_mst.update_output();
            axis_slv.update_output();
        }
        top->eval();
        tfp->dump(tick_count);
    }
    tfp->close();
    delete tfp;
    delete top;

    std::cout << "Simulation finished at cycle " << tick_count << std::endl;
    return 0;
}
