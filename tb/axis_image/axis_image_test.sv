//*****************************************************************************
// Copyright (C) 2025 WanderingKitsune. All rights reserved.
// SPDX-License-Identifier: MIT
// 
// File:        axis_image_test.sv
// Description: AXI4-Stream Image Test Module
// Note:        C++ DPI uses VIP headers under ../../src/axis_image/ (and ../../src/axis/; see Makefile -I).
// Repository:  https://github.com/WanderingKitsune/vaxivip
//*****************************************************************************

`timescale 1ns / 1ps

module axis_image_test
#(
    parameter BPC = 8,
    parameter PPC = 4,
    parameter DATA_WIDTH = 3 * PPC * BPC,
    parameter KEEP_WIDTH = DATA_WIDTH / 8,
    parameter USER_WIDTH = 1
)
(
    input  logic                  axis_rst,
    input  logic                  axis_clk,

    input  logic [DATA_WIDTH-1:0] s_tdata,
    input  logic [KEEP_WIDTH-1:0] s_tkeep,
    input  logic [KEEP_WIDTH-1:0] s_tstrb,
    input  logic [0:0]            s_tid,
    input  logic                  s_tdest,
    input  logic [USER_WIDTH-1:0] s_tuser,
    input  logic                  s_tlast,
    input  logic                  s_tvalid,
    output logic                  s_tready,

    output logic [DATA_WIDTH-1:0] m_tdata,
    output logic [KEEP_WIDTH-1:0] m_tkeep,
    output logic [KEEP_WIDTH-1:0] m_tstrb,
    output logic [0:0]            m_tid,
    output logic                  m_tdest,
    output logic [USER_WIDTH-1:0] m_tuser,
    output logic                  m_tlast,
    output logic                  m_tvalid,
    input  logic                  m_tready
);

assign m_tdata = s_tdata;
assign m_tkeep = s_tkeep;
assign m_tstrb = s_tstrb;
assign m_tid = s_tid;
assign m_tdest = s_tdest;
assign m_tuser = s_tuser;
assign m_tlast = s_tlast;
assign m_tvalid = s_tvalid;
assign s_tready = m_tready;

endmodule
