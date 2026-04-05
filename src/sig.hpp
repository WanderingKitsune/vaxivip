/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        sig.hpp
 * @brief       Signal type definitions for Verilator simulation
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     This header defines macros for declaring Verilator signal types
 *              based on bit width.
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0   wk   2025/12/25  Initial release
 ******************************************************************************/


#ifndef SIG_HPP
#define SIG_HPP

#include <verilated.h>
#include <condition_variable>

#define sig_t(msb, lsb) \
    typename std::conditional<((msb+1)-(lsb)) <= 8,  CData, \
    typename std::conditional<((msb+1)-(lsb)) <= 16, SData, \
    typename std::conditional<((msb+1)-(lsb)) <= 32, IData, \
    typename std::conditional<((msb+1)-(lsb)) <= 64, QData, \
    VlWide<(((msb+1)-(lsb))+31)/32>>::type>::type>::type>::type

#define sig_io(name, msb, lsb) sig_t(msb,lsb) name
#define sig_in(name, msb, lsb) const sig_t(msb, lsb) name
#define sig_out(name, msb, lsb) sig_t(msb, lsb) name

#endif
