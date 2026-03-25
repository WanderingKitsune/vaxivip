//*****************************************************************************
// Copyright (C) 2025 WanderingKitsune. All rights reserved.
// SPDX-License-Identifier: MIT
// 
// File:        axil_test.sv
// Description: AXI4-Lite Test Module
// Note:        C++ DPI uses VIP headers under ../../src/axil/ (see tb/axil/Makefile -I).
// Repository:  https://github.com/WanderingKitsune/vaxivip
// 
// Modification History:
// Ver   Who       Date        Changes
// ----  ----  ----------  ----------------------------------------------------
// 1.0         2025/12/25  Initial release
//*****************************************************************************

`timescale 1ns / 1ps

module axil_test
#(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 16,
    parameter STRB_WIDTH = (DATA_WIDTH/8)
)
(
    input  wire                   clk,
    input  wire                   rst,

    input  wire [ADDR_WIDTH-1:0]  s_axil_awaddr,
    input  wire [2:0]             s_axil_awprot,
    input  wire                   s_axil_awvalid,
    output wire                   s_axil_awready,
    input  wire [DATA_WIDTH-1:0]  s_axil_wdata,
    input  wire [STRB_WIDTH-1:0]  s_axil_wstrb,
    input  wire                   s_axil_wvalid,
    output wire                   s_axil_wready,
    output wire [1:0]             s_axil_bresp,
    output wire                   s_axil_bvalid,
    input  wire                   s_axil_bready,
    input  wire [ADDR_WIDTH-1:0]  s_axil_araddr,
    input  wire [2:0]             s_axil_arprot,
    input  wire                   s_axil_arvalid,
    output wire                   s_axil_arready,
    output wire [DATA_WIDTH-1:0]  s_axil_rdata,
    output wire [1:0]             s_axil_rresp,
    output wire                   s_axil_rvalid,
    input  wire                   s_axil_rready,

    output wire [ADDR_WIDTH-1:0]  m_axil_awaddr,
    output wire [2:0]             m_axil_awprot,
    output wire                   m_axil_awvalid,
    input  wire                   m_axil_awready,
    output wire [DATA_WIDTH-1:0]  m_axil_wdata,
    output wire [STRB_WIDTH-1:0]  m_axil_wstrb,
    output wire                   m_axil_wvalid,
    input  wire                   m_axil_wready,
    input  wire [1:0]             m_axil_bresp,
    input  wire                   m_axil_bvalid,
    output wire                   m_axil_bready,
    output wire [ADDR_WIDTH-1:0]  m_axil_araddr,
    output wire [2:0]             m_axil_arprot,
    output wire                   m_axil_arvalid,
    input  wire                   m_axil_arready,
    input  wire [DATA_WIDTH-1:0]  m_axil_rdata,
    input  wire [1:0]             m_axil_rresp,
    input  wire                   m_axil_rvalid,
    output wire                   m_axil_rready
);

assign m_axil_awaddr  = s_axil_awaddr;
assign m_axil_awprot  = s_axil_awprot;
assign m_axil_awvalid = s_axil_awvalid;
assign s_axil_awready = m_axil_awready;

assign m_axil_wdata   = s_axil_wdata;
assign m_axil_wstrb   = s_axil_wstrb;
assign m_axil_wvalid  = s_axil_wvalid;
assign s_axil_wready  = m_axil_wready;

assign s_axil_bresp   = m_axil_bresp;
assign s_axil_bvalid  = m_axil_bvalid;
assign m_axil_bready  = s_axil_bready;

assign m_axil_araddr  = s_axil_araddr;
assign m_axil_arprot  = s_axil_arprot;
assign m_axil_arvalid = s_axil_arvalid;
assign s_axil_arready = m_axil_arready;

assign s_axil_rdata   = m_axil_rdata;
assign s_axil_rresp   = m_axil_rresp;
assign s_axil_rvalid  = m_axil_rvalid;
assign m_axil_rready  = s_axil_rready;

endmodule
