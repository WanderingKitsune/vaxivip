//*****************************************************************************
// Copyright (C) 2025 WanderingKitsune. All rights reserved.
// SPDX-License-Identifier: MIT
// 
// File:        axis_test.sv
// Description: AXI4-Stream Test Module
// Note:        C++ DPI uses VIP headers under ../../src/axis/ (see tb/axis/Makefile -I).
// Repository:  https://github.com/WanderingKitsune/vaxivip
// 
// Modification History:
// Ver   Who       Date        Changes
// ----  ----  ----------  ----------------------------------------------------
// 1.0         2025/12/25  Initial release
//*****************************************************************************

`timescale 1ns / 1ps

module axis_test(
   input  logic                  axis_rst,
   input  logic                  axis_clk,

   input  logic [255:0]          s_tdata,
   input  logic [31:0]           s_tkeep,
   input  logic                  s_tdest,
   input  logic [31:0]           s_tstrb,
   input  logic [7:0]            s_tid,
   input  logic                  s_tuser,
   input  logic                  s_tlast,
   input  logic                  s_tvalid,
   output logic                  s_tready,

   output logic [255:0]          m_tdata,
   output logic [31:0]           m_tkeep,
   output logic                  m_tdest,
   output logic [31:0]           m_tstrb,
   output logic [7:0]            m_tid,
   output logic                  m_tuser,
   output logic                  m_tlast,
   output logic                  m_tvalid,
   input  logic                  m_tready
);

assign m_tdata = s_tdata;
assign m_tkeep = s_tkeep;
assign m_tdest = s_tdest;
assign m_tstrb = s_tstrb;
assign m_tid = s_tid;
assign m_tuser = s_tuser;
assign m_tlast = s_tlast;
assign m_tvalid = s_tvalid;
assign s_tready = m_tready;

endmodule
