/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_image_bfm.hpp
 * @brief       AXI4-Stream BFM for axis_image_vip (master/slave)
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     AXI4-Stream BFM for master/slave interfaces with configurable
 *              data width, ID, destination, and user widths.
 *
 * @ingroup axis_image_vip
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/30  Initial release
 ******************************************************************************/

#ifndef AXIS_IMAGE_BFM_HPP
#define AXIS_IMAGE_BFM_HPP

#include "axis.hpp"
#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>

/**
 * @brief AXI4-Stream master BFM for image data streaming
 * @tparam DATA_WIDTH TDATA width in bits (default: 64)
 * @tparam ID_WIDTH TID width in bits (default: 8)
 * @tparam DEST_WIDTH TDEST width in bits (default: 1)
 * @tparam USER_WIDTH TUSER width in bits (default: 1)
 */
template <
    uint32_t DATA_WIDTH = 64,
    uint32_t ID_WIDTH = 8,
    uint32_t DEST_WIDTH = 1,
    uint32_t USER_WIDTH = 1
>
class axis_image_bfm_mst {
public:
    std::vector<uint8_t>              tx_buf;        ///< Current transaction payload buffer
    size_t                            tx_buf_idx = 0;///< Current byte index in tx_buf
    std::queue<std::vector<uint8_t>>  tx_queue;      ///< Pending transactions payload queue
    std::queue<uint32_t>              tx_id_queue;   ///< Pending TID queue
    std::queue<uint32_t>              tx_dest_queue; ///< Pending TDEST queue
    std::queue<uint32_t>              tx_user_queue; ///< Pending TUSER queue (bit0 used for SOF)
    std::queue<bool>                  tx_sof_queue;  ///< Pending SOF flags (mapped to TUSER[0] on first beat)
    axis_master_ptr<DATA_WIDTH, ID_WIDTH, DEST_WIDTH, USER_WIDTH> port; ///< AXIS master port pointers
    int         byte_width;                           ///< DATA_WIDTH in bytes
    uint32_t    tx_id   = 0;                          ///< Current TID
    uint32_t    tx_dest = 0;                          ///< Current TDEST
    uint32_t    tx_user = 0;                          ///< Current TUSER
    bool        tx_sof  = false;                      ///< Current SOF flag
    bool        tready_i = false;                     ///< Registered input TREADY

    /// @brief Constructor
    /// @param p AXI4-Stream master interface pointer
    explicit axis_image_bfm_mst(axis_master_ptr<DATA_WIDTH, ID_WIDTH, DEST_WIDTH, USER_WIDTH> p)
        : port(p) {
        byte_width = static_cast<int>(DATA_WIDTH / 8);
    }

    /// @brief Enqueue one transaction (tlast asserted at transaction end)
    /// @param data Payload byte vector
    /// @param id Transaction ID (TID)
    /// @param dest Destination (TDEST)
    /// @param user User data (TUSER)
    /// @param sof Start of Frame flag (maps to TUSER[0] on first beat)
    void send(const std::vector<uint8_t>& data, uint32_t id = 0, uint32_t dest = 0,
              uint32_t user = 0, bool sof = false) {
        tx_queue.push(data);
        tx_id_queue.push(id);
        tx_dest_queue.push(dest);
        tx_user_queue.push(user);
        tx_sof_queue.push(sof);
    }

    /// @brief Sample registered inputs from DUT
    void update_input() {
        tready_i = *(port.tready);
    }

    /// @brief Drive outputs to DUT on cycle
    void update_output() {
        if (!tready_i) return;
        *(port.tvalid) = false;
        if (tx_buf.empty() && !tx_queue.empty()) {
            tx_buf = tx_queue.front();
            tx_queue.pop();
            if (!tx_id_queue.empty()) {
                tx_id = tx_id_queue.front();
                tx_id_queue.pop();
            }
            if (!tx_dest_queue.empty()) {
                tx_dest = tx_dest_queue.front();
                tx_dest_queue.pop();
            }
            if (!tx_user_queue.empty()) {
                tx_user = tx_user_queue.front();
                tx_user_queue.pop();
            }
            if (!tx_sof_queue.empty()) {
                tx_sof = tx_sof_queue.front();
                tx_sof_queue.pop();
            } else {
                tx_sof = false;
            }
            tx_buf_idx = 0;
        }
        if (tx_buf.empty()) return;
        int byte_pos = 0;
        *(port.tkeep) = 0;
        *(port.tid)   = tx_id;
        *(port.tdest) = tx_dest;
        if (tx_buf_idx == 0 && tx_sof)
            *(port.tuser) = tx_user | 0x1u;
        else
            *(port.tuser) = tx_user;
        while (tx_buf_idx < tx_buf.size() && byte_pos < byte_width) {
            *(port.tkeep) = *(port.tkeep) | (static_cast<uint64_t>(1) << byte_pos);
            (reinterpret_cast<char*>(port.tdata))[byte_pos] =
                static_cast<char>(tx_buf[tx_buf_idx]);
            tx_buf_idx++;
            byte_pos++;
        }
        bool last = (tx_buf_idx >= tx_buf.size());
        *(port.tlast)  = last;
        *(port.tvalid) = true;
        if (last) {
            tx_buf.clear();
            tx_buf_idx = 0;
        }
    }
};

/**
 * @brief AXI4-Stream slave BFM for image data reception
 * @tparam DATA_WIDTH TDATA width in bits (default: 64)
 * @tparam ID_WIDTH TID width in bits (default: 8)
 * @tparam DEST_WIDTH TDEST width in bits (default: 1)
 * @tparam USER_WIDTH TUSER width in bits (default: 1)
 */
template <
    uint32_t DATA_WIDTH = 64,
    uint32_t ID_WIDTH = 8,
    uint32_t DEST_WIDTH = 1,
    uint32_t USER_WIDTH = 1
>
class axis_image_bfm_slv {
public:
    std::vector<uint8_t>              recv_buf;   ///< Current assembling transaction buffer
    std::queue<std::vector<uint8_t>>  rx_queue;   ///< Completed transactions queue
    axis_slave_ptr<DATA_WIDTH, ID_WIDTH, DEST_WIDTH, USER_WIDTH> port; ///< AXIS slave port pointers
    bool     tready_o = true;                     ///< Driven output TREADY
    bool     tvalid_i = false;                    ///< Registered input TVALID
    uint64_t tkeep_i  = 0;                        ///< Registered input TKEEP
    bool     tlast_i  = false;                    ///< Registered input TLAST
    uint32_t tuser_i  = 0;                        ///< Registered input TUSER
    std::vector<uint8_t> tdata_i;                 ///< Registered input TDATA (byte array)

    /// @brief Constructor
    /// @param p AXI4-Stream slave interface pointer
    explicit axis_image_bfm_slv(axis_slave_ptr<DATA_WIDTH, ID_WIDTH, DEST_WIDTH, USER_WIDTH> p)
        : port(p) {
        *(port.tready) = tready_o;
    }

    /// @brief Check if receive queue is empty
    /// @return true if no completed transactions are queued, false otherwise
    bool empty() const {
        return rx_queue.empty();
    }

    /// @brief Pop one completed transaction payload
    /// @param dst_buf Destination buffer to receive the payload
    /// @return Size of payload in bytes, or -1 if queue is empty
    ssize_t recv(std::vector<uint8_t>& dst_buf) {
        if (rx_queue.empty()) return -1;
        dst_buf = rx_queue.front();
        rx_queue.pop();
        return static_cast<ssize_t>(dst_buf.size());
    }

    /// @brief Set TREADY drive value
    /// @param ready TREADY value to drive (true = ready, false = not ready)
    void set_tready(bool ready) {
        tready_o = ready;
    }

    /// @brief Sample registered inputs from DUT
    void update_input() {
        tvalid_i = *(port.tvalid);
        tkeep_i  = *(port.tkeep);
        tlast_i  = *(port.tlast);
        tuser_i  = *(port.tuser);
        tdata_i.clear();
        for (int i = 0; i < static_cast<int>(DATA_WIDTH / 8); i++)
            tdata_i.push_back((reinterpret_cast<const char*>(port.tdata))[i]);
    }

    /// @brief Drive outputs to DUT and assemble payload into rx_queue
    void update_output() {
        if (tvalid_i) {
            for (int i = 0; i < static_cast<int>(DATA_WIDTH / 8); i++) {
                if ((tkeep_i & (static_cast<uint64_t>(1) << i)) != 0)
                    recv_buf.push_back(tdata_i[static_cast<size_t>(i)]);
            }
            if (tlast_i) {
                rx_queue.push(recv_buf);
                recv_buf.clear();
            }
        }
        *(port.tready) = tready_o;
    }
};

#endif
