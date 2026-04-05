/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        pix_fmt.hpp
 * @brief       Pixel format definitions for video processing
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     Defines pixel formats used in YUV planar video processing.
 *
 * @ingroup axis_video
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2026/04/05  Initial release
 ******************************************************************************/

#ifndef PIX_FMT_HPP
#define PIX_FMT_HPP

#include <cstdint>

/**
 * @brief Pixel format enumeration
 * @details Defines supported pixel formats for video processing.
 */
enum PixlFmt {
    PIX_FMT_NONE = -1,
    PIX_FMT_YUV444P = 0
};

/** @} */ // end of group axis_video

#endif
