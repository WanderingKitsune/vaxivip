//*****************************************************************************
// Copyright (C) 2025 WanderingKitsune. All rights reserved.
// SPDX-License-Identifier: MIT
//
// File:        axi_test.sv
// Description: AXI4 Test Module
// Note:        C++ DPI uses VIP headers under ../../src/axi/ (see tb/axi/Makefile -I).
// Repository:  https://github.com/WanderingKitsune/vaxivip
//
// Modification History:
// Ver   Who       Date        Changes
// ----  ----  ----------  ----------------------------------------------------
// 1.0         2025/12/26  Initial release
//*****************************************************************************

`timescale 1ns / 1ps

module axi_test
#(
   parameter ADDR_WIDTH                 = 40,          // Address width
   parameter DATA_WIDTH                 = 256,         // Data width
   parameter ID_WIDTH                   = 16,          // ID width
   parameter STRB_WIDTH                 = DATA_WIDTH/8 // Strobe width
)
(
    input  wire                         clk,
    input  wire                         rst,

    // Slave Interface (Input to DUT, driven by Master BFM)
    input  wire [ADDR_WIDTH-1:0]        s_axi_awaddr,
    input  wire [1:0]                   s_axi_awburst,
    input  wire [3:0]                   s_axi_awcache,
    input  wire [ID_WIDTH-1:0]          s_axi_awid,
    input  wire [7:0]                   s_axi_awlen,
    input  wire                         s_axi_awlock,
    input  wire [2:0]                   s_axi_awprot,
    input  wire [3:0]                   s_axi_awqos,
    output wire                         s_axi_awready,
    input  wire [3:0]                   s_axi_awregion,
    input  wire [2:0]                   s_axi_awsize,
    input  wire                         s_axi_awvalid,

    input  wire [DATA_WIDTH-1:0]        s_axi_wdata,
    input  wire [ID_WIDTH-1:0]          s_axi_wid,
    input  wire                         s_axi_wlast,
    output wire                         s_axi_wready,
    input  wire [STRB_WIDTH-1:0]        s_axi_wstrb,
    input  wire                         s_axi_wvalid,

    output wire [ID_WIDTH-1:0]          s_axi_bid,
    input  wire                         s_axi_bready,
    output wire [1:0]                   s_axi_bresp,
    output wire                         s_axi_bvalid,

    input  wire [ADDR_WIDTH-1:0]        s_axi_araddr,
    input  wire [1:0]                   s_axi_arburst,
    input  wire [3:0]                   s_axi_arcache,
    input  wire [ID_WIDTH-1:0]          s_axi_arid,
    input  wire [7:0]                   s_axi_arlen,
    input  wire                         s_axi_arlock,
    input  wire [2:0]                   s_axi_arprot,
    input  wire [3:0]                   s_axi_arqos,
    output wire                         s_axi_arready,
    input  wire [3:0]                   s_axi_arregion,
    input  wire [2:0]                   s_axi_arsize,
    input  wire                         s_axi_arvalid,

    output wire [DATA_WIDTH-1:0]        s_axi_rdata,
    output wire [ID_WIDTH-1:0]          s_axi_rid,
    output wire                         s_axi_rlast,
    input  wire                         s_axi_rready,
    output wire [1:0]                   s_axi_rresp,
    output wire                         s_axi_rvalid,

    // Master Interface (Output from DUT, connected to Slave BFM)
    output wire [ADDR_WIDTH-1:0]        m_axi_awaddr,
    output wire [1:0]                   m_axi_awburst,
    output wire [3:0]                   m_axi_awcache,
    output wire [ID_WIDTH-1:0]          m_axi_awid,
    output wire [7:0]                   m_axi_awlen,
    output wire                         m_axi_awlock,
    output wire [2:0]                   m_axi_awprot,
    output wire [3:0]                   m_axi_awqos,
    input  wire                         m_axi_awready,
    output wire [3:0]                   m_axi_awregion,
    output wire [2:0]                   m_axi_awsize,
    output wire                         m_axi_awvalid,

    output wire [DATA_WIDTH-1:0]        m_axi_wdata,
    output wire [ID_WIDTH-1:0]          m_axi_wid,
    output wire                         m_axi_wlast,
    input  wire                         m_axi_wready,
    output wire [STRB_WIDTH-1:0]        m_axi_wstrb,
    output wire                         m_axi_wvalid,

    input  wire [ID_WIDTH-1:0]          m_axi_bid,
    output wire                         m_axi_bready,
    input  wire [1:0]                   m_axi_bresp,
    input  wire                         m_axi_bvalid,

    output wire [ADDR_WIDTH-1:0]        m_axi_araddr,
    output wire [1:0]                   m_axi_arburst,
    output wire [3:0]                   m_axi_arcache,
    output wire [ID_WIDTH-1:0]          m_axi_arid,
    output wire [7:0]                   m_axi_arlen,
    output wire                         m_axi_arlock,
    output wire [2:0]                   m_axi_arprot,
    output wire [3:0]                   m_axi_arqos,
    input  wire                         m_axi_arready,
    output wire [3:0]                   m_axi_arregion,
    output wire [2:0]                   m_axi_arsize,
    output wire                         m_axi_arvalid,

    input  wire [DATA_WIDTH-1:0]        m_axi_rdata,
    input  wire [ID_WIDTH-1:0]          m_axi_rid,
    input  wire                         m_axi_rlast,
    output wire                         m_axi_rready,
    input  wire [1:0]                   m_axi_rresp,
    input  wire                         m_axi_rvalid
);

// Pass-through connection
assign m_axi_awaddr   = s_axi_awaddr;
assign m_axi_awburst  = s_axi_awburst;
assign m_axi_awcache  = s_axi_awcache;
assign m_axi_awid     = s_axi_awid;
assign m_axi_awlen    = s_axi_awlen;
assign m_axi_awlock   = s_axi_awlock;
assign m_axi_awprot   = s_axi_awprot;
assign m_axi_awqos    = s_axi_awqos;
assign s_axi_awready  = m_axi_awready;
assign m_axi_awregion = s_axi_awregion;
assign m_axi_awsize   = s_axi_awsize;
assign m_axi_awvalid  = s_axi_awvalid;

assign m_axi_wdata    = s_axi_wdata;
assign m_axi_wid      = s_axi_wid;
assign m_axi_wlast    = s_axi_wlast;
assign s_axi_wready   = m_axi_wready;
assign m_axi_wstrb    = s_axi_wstrb;
assign m_axi_wvalid   = s_axi_wvalid;

assign s_axi_bid      = m_axi_bid;
assign m_axi_bready   = s_axi_bready;
assign s_axi_bresp    = m_axi_bresp;
assign s_axi_bvalid   = m_axi_bvalid;

assign m_axi_araddr   = s_axi_araddr;
assign m_axi_arburst  = s_axi_arburst;
assign m_axi_arcache  = s_axi_arcache;
assign m_axi_arid     = s_axi_arid;
assign m_axi_arlen    = s_axi_arlen;
assign m_axi_arlock   = s_axi_arlock;
assign m_axi_arprot   = s_axi_arprot;
assign m_axi_arqos    = s_axi_arqos;
assign s_axi_arready  = m_axi_arready;
assign m_axi_arregion = s_axi_arregion;
assign m_axi_arsize   = s_axi_arsize;
assign m_axi_arvalid  = s_axi_arvalid;

assign s_axi_rdata    = m_axi_rdata;
assign s_axi_rid      = m_axi_rid;
assign s_axi_rlast    = m_axi_rlast;
assign m_axi_rready   = s_axi_rready;
assign s_axi_rresp    = m_axi_rresp;
assign s_axi_rvalid   = m_axi_rvalid;

endmodule
