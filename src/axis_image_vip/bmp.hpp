/******************************************************************************
 * Copyright (C) 2025 WanderingKitsune. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * @file        bmp.hpp
 * @brief       BMP Image Processing Class
 * @see         https://github.com/WanderingKitsune/vaxivip
 *
 * @details     BMP image read/write support for 24-bit and 32-bit uncompressed
 *              formats with bottom‑up pixel ordering.
 *
 * @ingroup axis_image_vip
 *
 * Modification History:
 * Ver   Who  Date        Changes
 * ----  ---- ----------  -----------------------------------------------------
 * 1.0        2025/12/30  Initial release
 ******************************************************************************/

#ifndef BMP_HPP
#define BMP_HPP

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

/**
 * @brief BMP image container with read/write capabilities
 * @details Stores pixel data in ARGB format (8 bits per channel, alpha channel optional).
 *          Supports 24‑bit and 32‑bit uncompressed BMP files with bottom‑up row order.
 */
class Bitmap {
public:
    uint32_t width;                            ///< Image width in pixels
    uint32_t height;                           ///< Image height in pixels
    uint16_t bit_count;                        ///< Bits per pixel (24 or 32)
    std::vector<std::vector<uint32_t>> data;   ///< Pixel data [height][width] in ARGB format

    /// @brief Default constructor
    Bitmap() : width(0), height(0), bit_count(0) {}

    /// @brief Read BMP file from disk
    /// @param filename Path to BMP file
    /// @return true if file read successfully, false otherwise
    bool read(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        uint16_t signature;
        file.read(reinterpret_cast<char*>(&signature), sizeof(signature));
        if (signature != 0x4D42) {
            return false;
        }

        uint32_t file_size, data_offset;
        file.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));
        file.seekg(10, std::ios::beg);
        file.read(reinterpret_cast<char*>(&data_offset), sizeof(data_offset));

        uint32_t bi_size, bi_width, bi_height;
        uint16_t bi_planes, bi_bit_count;
        uint32_t bi_compression;

        file.read(reinterpret_cast<char*>(&bi_size), sizeof(bi_size));
        file.read(reinterpret_cast<char*>(&bi_width), sizeof(bi_width));
        file.read(reinterpret_cast<char*>(&bi_height), sizeof(bi_height));
        file.read(reinterpret_cast<char*>(&bi_planes), sizeof(bi_planes));
        file.read(reinterpret_cast<char*>(&bi_bit_count), sizeof(bi_bit_count));
        file.read(reinterpret_cast<char*>(&bi_compression), sizeof(bi_compression));

        width = bi_width;
        height = bi_height > 0 ? bi_height : -bi_height;
        bit_count = bi_bit_count;

        if (bit_count != 24 && bit_count != 32) {
            return false;
        }

        if (bi_compression != 0) {
            return false;
        }

        data.resize(height);
        for (auto& row : data) {
            row.resize(width);
        }

        file.seekg(data_offset, std::ios::beg);

        if (bit_count == 32) {
            for (int y = height - 1; y >= 0; y--) {
                for (uint32_t x = 0; x < width; x++) {
                    uint8_t b, g, r, a;
                    file.read(reinterpret_cast<char*>(&b), 1);
                    file.read(reinterpret_cast<char*>(&g), 1);
                    file.read(reinterpret_cast<char*>(&r), 1);
                    file.read(reinterpret_cast<char*>(&a), 1);
                    data[y][x] = (a << 24) | (r << 16) | (g << 8) | b;
                }
            }
        } else {
            for (int y = height - 1; y >= 0; y--) {
                for (uint32_t x = 0; x < width; x++) {
                    uint8_t b, g, r;
                    file.read(reinterpret_cast<char*>(&b), 1);
                    file.read(reinterpret_cast<char*>(&g), 1);
                    file.read(reinterpret_cast<char*>(&r), 1);
                    data[y][x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                }
                uint32_t padding = (4 - (width * 3) % 4) % 4;
                file.seekg(padding, std::ios::cur);
            }
        }

        file.close();
        return true;
    }

    /// @brief Write BMP file to disk
    /// @param filename Path to output BMP file
    /// @return true if file written successfully, false otherwise
    bool write(const std::string& filename) const {
        if (width == 0 || height == 0) {
            return false;
        }

        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        uint16_t signature = 0x4D42;
        file.write(reinterpret_cast<const char*>(&signature), sizeof(signature));

        uint32_t row_size = ((width * 3 + 3) / 4) * 4;
        uint32_t image_size = row_size * height;
        uint32_t file_size = 54 + image_size;

        file.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
        uint32_t reserved = 0;
        file.write(reinterpret_cast<const char*>(&reserved), sizeof(reserved));
        uint32_t data_offset = 54;
        file.write(reinterpret_cast<const char*>(&data_offset), sizeof(data_offset));

        uint32_t bi_size = 40;
        file.write(reinterpret_cast<const char*>(&bi_size), sizeof(bi_size));
        int32_t bi_width = width;
        file.write(reinterpret_cast<const char*>(&bi_width), sizeof(bi_width));
        int32_t bi_height = height;
        file.write(reinterpret_cast<const char*>(&bi_height), sizeof(bi_height));
        uint16_t bi_planes = 1;
        file.write(reinterpret_cast<const char*>(&bi_planes), sizeof(bi_planes));
        uint16_t bi_bit_count = 24;
        file.write(reinterpret_cast<const char*>(&bi_bit_count), sizeof(bi_bit_count));
        uint32_t bi_compression = 0;
        file.write(reinterpret_cast<const char*>(&bi_compression), sizeof(bi_compression));
        file.write(reinterpret_cast<const char*>(&image_size), sizeof(image_size));
        int32_t bi_x_pels_per_meter = 0;
        file.write(reinterpret_cast<const char*>(&bi_x_pels_per_meter), sizeof(bi_x_pels_per_meter));
        int32_t bi_y_pels_per_meter = 0;
        file.write(reinterpret_cast<const char*>(&bi_y_pels_per_meter), sizeof(bi_y_pels_per_meter));
        uint32_t bi_clr_used = 0;
        file.write(reinterpret_cast<const char*>(&bi_clr_used), sizeof(bi_clr_used));
        uint32_t bi_clr_important = 0;
        file.write(reinterpret_cast<const char*>(&bi_clr_important), sizeof(bi_clr_important));

        for (int y = height - 1; y >= 0; y--) {
            for (uint32_t x = 0; x < width; x++) {
                uint32_t pixel = data[y][x];
                uint8_t b = pixel & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t r = (pixel >> 16) & 0xFF;
                file.write(reinterpret_cast<const char*>(&b), 1);
                file.write(reinterpret_cast<const char*>(&g), 1);
                file.write(reinterpret_cast<const char*>(&r), 1);
            }
            uint32_t padding = (4 - (width * 3) % 4) % 4;
            uint8_t pad_byte = 0;
            for (uint32_t p = 0; p < padding; p++) {
                file.write(reinterpret_cast<const char*>(&pad_byte), 1);
            }
        }

        file.close();
        return true;
    }

    /// @brief Get pixel color at specified coordinates
    /// @param x X coordinate (0‑based, left to right)
    /// @param y Y coordinate (0‑based, top to bottom)
    /// @return ARGB color value (0xAARRGGBB), or 0 if out of bounds
    uint32_t get_pixel(uint32_t x, uint32_t y) const {
        if (x >= width || y >= height) {
            return 0;
        }
        return data[y][x];
    }

    /// @brief Set pixel color at specified coordinates
    /// @param x X coordinate (0‑based, left to right)
    /// @param y Y coordinate (0‑based, top to bottom)
    /// @param color ARGB color value (0xAARRGGBB)
    void set_pixel(uint32_t x, uint32_t y, uint32_t color) {
        if (x >= width || y >= height) {
            return;
        }
        data[y][x] = color;
    }

    /// @brief Create a new blank image with specified dimensions
    /// @param w Image width in pixels
    /// @param h Image height in pixels
    void create(uint32_t w, uint32_t h) {
        width = w;
        height = h;
        bit_count = 24;
        data.resize(height);
        for (auto& row : data) {
            row.resize(width, 0xFF000000);
        }
    }
};

#endif
