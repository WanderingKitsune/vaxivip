/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_vip.hpp
 * @brief       AXI4-Stream VIP (Verification IP)
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     This module implements the VIP for AXI4-Stream protocol verification.
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/30  Initial release (Separated from axis.hpp)
 ******************************************************************************/

#ifndef AXIS_VIP_HPP
#define AXIS_VIP_HPP

#include "axis.hpp"
#include "log.hpp"
#include <cstring>
#include <queue>

/// @brief AXI4-Stream Master BFM
template <uint32_t DATA_WIDTH = 64>
class axis_master {
public:
    Log log;
    std::vector<uint8_t> tx_buf;            ///< Current transaction buffer
    size_t tx_buf_idx;                      ///< Current index in tx_buf
    std::queue<std::vector<uint8_t>> tx_queue;///< Queue of pending transactions
    axis_master_ptr<DATA_WIDTH> port;       ///< Interface signal pointers
    bool tx_tuser;                          ///< TUSER value for current transaction
    int byte_width;                         ///< Data width in bytes
    
    // Registered Input Signals
    bool tready_i;

    /// @brief Constructor
    axis_master(axis_master_ptr<DATA_WIDTH> port):port(port) {
        tx_tuser = false;
        byte_width = DATA_WIDTH/8;
        tx_buf_idx = 0;
        tready_i = false;
    }

    /// @brief Destructor
    ~axis_master() = default;

    /// @brief Send data through AXI Stream
    /// @param with_tuser Assert TUSER (e.g. for frame start/end indication)
    void send(const char *src_buf, ssize_t size, bool with_tuser = false) {
        tx_queue.emplace(src_buf, src_buf + size);
        if ( with_tuser ) tx_tuser = true;
    }

    /// @brief Set the TUSER signal value
    void set_tuser(uint32_t src_tuser) {
        *(port.tuser) = src_tuser;
    }

    /// @brief Get the current TREADY state
    bool get_tready() {
        return tready_i;
    }

    /// @brief Cycle tick
    void update_input() {
        tready_i = *(port.tready);
    }

    void update_output() {
        if (tready_i) {
            *(port.tvalid) = false;
            // start new transaction
            if (tx_buf.empty() && !tx_queue.empty()) {
                tx_buf = tx_queue.front();
                tx_queue.pop();
                tx_buf_idx = 0;
            }
            if (!tx_buf.empty()) {
                int byte_pos = 0;
                *(port.tkeep) = 0;
                while (tx_buf_idx < tx_buf.size() && byte_pos < byte_width) {
                    *(port.tkeep) = *(port.tkeep) | ((uint64_t)1 << byte_pos);
                    ((char*)port.tdata)[byte_pos] = tx_buf[tx_buf_idx];
                    tx_buf_idx++;
                    byte_pos++;
                }
                if (tx_tuser) {
                    *(port.tuser) = *(port.tuser) | 0x1;
                    tx_tuser = false;
                } else {
                    *(port.tuser) = *(port.tuser) & (~0x1);
                }
                
                bool last = (tx_buf_idx >= tx_buf.size());
                *(port.tlast) = last;
                *(port.tvalid) = true;

                if (last) {
                    log.info("[AXIS-MST] SEND success !");
                    log.info("SIZE:", std::dec, tx_buf.size());
                    log.hexdump(tx_buf, 0);
                    tx_buf.clear();
                    tx_buf_idx = 0;
                }
            }
        }
    }
};

/// @brief AXI4-Stream Slave BFM
template <uint32_t DATA_WIDTH = 64>
class axis_slave {
public:
    Log log;
    std::vector<uint8_t> recv_buf;          ///< Buffer for currently receiving packet
    std::queue<std::vector<uint8_t>> rx_queue;///< Queue of received packets
    axis_slave_ptr<DATA_WIDTH> port;        ///< Interface signal pointers

    // Registered Input Signals
    bool tvalid_i;
    uint64_t tkeep_i;
    bool tlast_i;
    uint32_t tuser_i;
    std::vector<uint8_t> tdata_i;

    /// @brief Constructor
    axis_slave(axis_slave_ptr<DATA_WIDTH> port):port(port) {
        *(port.tready) = true; // always tready
        tvalid_i = false;
        tkeep_i = 0;
        tlast_i = false;
        tuser_i = 0;
    }

    /// @brief Destructor
    ~axis_slave() = default;

    /// @brief Check if there is valid data received
    bool has_valid_data() {
        return !rx_queue.empty();
    }

    /// @brief Receive data from the internal queue
    /// @return Number of bytes actually read, or -1 if empty
    ssize_t recv(int max_size, char *dst_buf) {
        if (rx_queue.empty()) {
            return -1;
        } else {
            std::vector<uint8_t>& data = rx_queue.front();
            size_t copy_size = (data.size() <= (size_t)max_size) ? data.size() : (size_t)max_size;
            for (size_t i = 0; i < copy_size; i++) {
                dst_buf[i] = (char)data[i];
            }
            rx_queue.pop();
            return copy_size;
        }
    }

    /// @brief Cycle tick
    void update_input() {
        tvalid_i = *(port.tvalid);
        tkeep_i = *(port.tkeep);
        tlast_i = *(port.tlast);
        tuser_i = *(port.tuser);
        
        tdata_i.clear();
        for (int i=0; i<DATA_WIDTH/8; i++) {
             tdata_i.push_back(((char*)port.tdata)[i]);
        }
    }

    void update_output() {
        if (tvalid_i) {
            for (int i=0;i<DATA_WIDTH/8;i++) {
                if ((tkeep_i & ((uint64_t)1 << i)) != 0) {
                    recv_buf.push_back(tdata_i[i]);
                }
            }
            if (tlast_i) {
                rx_queue.push(recv_buf);
                log.info("[AXIS-SLV] RECV success !");
                log.info("SIZE:", std::dec, recv_buf.size());
                log.hexdump(recv_buf, 0);
                recv_buf.clear();
            }
        }
        // Drive tready (always true)
        *(port.tready) = true;
    }
};

#endif
