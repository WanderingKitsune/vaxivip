/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        axis_video_format.hpp
 * @brief       AXI Stream Video Format Definitions
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     Defines pixel formats, bits per channel, pixels per cycle,
 *              and validation macros for AXI4-Stream video interfaces.
 *
 * @ingroup axis_video
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/30  Initial release
 ******************************************************************************/

#ifndef AXIS_VIDEO_FORMAT_HPP
#define AXIS_VIDEO_FORMAT_HPP

#include <cstdint>

/**
 * @brief Axis pixel format types
 * 
 *  Pixel counter    max_ppc ...  pix1   pix0
 * AXIS_PIX_FMT_YUV:   V U Y ... V U Y  V U Y
 * AXIS_PIX_FMT_GBR:   R B G ... R B G  R B G
 * AXIS_PIX_FMT_YUYV:   V Y  ...  V Y    U Y 
 */
enum AxisPixFmt : uint32_t {
    AXIS_PIX_FMT_NONE = 0,
    AXIS_PIX_FMT_GBR  = 0,
    AXIS_PIX_FMT_YUV  = 1,
    AXIS_PIX_FMT_YUYV = 2
};

/**
 * @brief Bits per color
 */
enum BpcFmt : uint32_t {
    BPC_FMT_NONE = 0,
    BPC_FMT_8 = 8,
    BPC_FMT_10 = 10,
    BPC_FMT_12 = 12
};

/**
 * @brief Pixels per clock
 */
enum PpcFmt : uint32_t {
    PPC_FMT_NONE = 0,
    PPC_FMT_1 = 1,
    PPC_FMT_2 = 2,
    PPC_FMT_4 = 4
};

/**
 * @brief Static assertion macros for BPC and PPC validation
 */
#define static_assert_bpc(bpc) static_assert((bpc) == BPC_FMT_8 || (bpc) == BPC_FMT_10 || (bpc) == BPC_FMT_12, "BPC must be 8, 10, or 12")
#define static_assert_ppc(ppc) static_assert((ppc) == PPC_FMT_1 || (ppc) == PPC_FMT_2 || (ppc) == PPC_FMT_4, "PPC must be 1, 2, or 4")

/** @} */ // end of group axis_video

#endif
