![Language](https://img.shields.io/badge/Language-C++-f34b7d.svg) ![Simulation](https://img.shields.io/badge/Simulation-Verilator-007ec6.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg)

[English](#en) | [中文](#cn)

---

<span id="en">vaxivip</span>
===========================

**vaxivip** is a lightweight AXI protocol Verification IP (VIP) library for [Verilator](https://www.veripool.org/verilator/) simulation. Written in C++, it simulates AXI Master and Slave behaviors in C++ testbenches to verify Verilog/SystemVerilog designs.

## ✨ Features

| VIP Module | Protocol | Core Features | Typical Use Case |
|------------|----------|---------------|------------------|
| **`axi_vip`** | AXI4 | Memory-mapped read/write, burst transfers | Processor/memory bus verification |
| **`axil_vip`** | AXI4-Lite | Simple register access | Peripheral register access verification |
| **`axis_vip`** | AXI4-Stream | Stream data transfer | Video streaming, network packet transmission |
| **`axis_image_vip`** | AXI4-Stream | BMP image streaming with configurable BPC/PPC (Xilinx AXI4-Stream Video compliant) | Image processing IP verification (BMP I/O) |

## 🚀 Quick Start

### 1. Install Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install verilator g++ make

# macOS (with Homebrew)
brew install verilator
```

### 2. Clone and Explore
```bash
git clone https://github.com/WanderingKitsune/vaxivip.git
cd vaxivip
```



### 3. Compile and Run Tests
```bash
# Navigate to test directory
cd tb/axi

# Compile and run simulation
make

# View waveform (optional)
# gtkwave waveform.vcd

# Clean build artifacts
make clean
```

💡 **Tip**: Check other test directories (`tb/axil/`, `tb/axis/`, `tb/axis_image/`) for more examples!

## 📖 Detailed Usage Guide

### Project Structure
```
.
├── src/                # VIP source code
│   ├── axi_vip/        # AXI4 VIP (Master/Slave)
│   ├── axil_vip/       # AXI4-Lite VIP (Master/Slave)
│   ├── axis_vip/       # AXI4-Stream VIP (Master/Slave)
│   └── axis_image_vip/ # AXI4-Stream Image VIP (BMP support)
├── tb/                 # Testbenches and examples
│   ├── axi/            # AXI4 tests
│   ├── axil/           # AXI4-Lite tests
│   ├── axis/           # AXI4-Stream tests
│   └── axis_image/     # AXI4-Stream image tests
└── README.md
```

### 1. Include Headers
```cpp
#include "axi_vip/axi_vip.hpp"        // AXI4 VIP
#include "axil_vip/axil_vip.hpp"      // AXI4-Lite VIP  
#include "axis_vip/axis_vip.hpp"      // AXI4-Stream VIP
#include "axis_image_vip/axis_image_vip.hpp"  // AXI4-Stream Image VIP
```

### 2. Bind Signals
Bind Verilated model signal pointers to the VIP signal structure:
```cpp
// Define AXI bus structure (match your DUT parameters)
axi_ptr<256, 40, 16> axi_sig;  // DataWidth=256, AddrWidth=40, IdWidth=16

// Bind signals (point VIP pointers to Verilator model signals)
axi_sig.awaddr  = &(top->m_axi_awaddr);
axi_sig.awvalid = &(top->m_axi_awvalid);
axi_sig.awready = &(top->m_axi_awready);
// ... bind all required AXI signals
```

### 3. Create VIP Instances
```cpp
// Create AXI Master instance
axi_master<256, 40, 16> mst(axi_sig);

// Or create AXI Slave instance
axi_slave<256, 40, 16> slv(axi_sig);
```

### 4. Drive in Simulation Loop

**Important**: Call `update_input()` before `eval()` to correctly simulate sampling of input signals at clock edges.

```cpp
while (!Verilated::gotFinish()) {
    // ... clock generation ...
    
    if (top->clk) {
        // 1. Read input signals (sample at clock edge)
        mst.update_input();
        slv.update_input();
    }

    top->eval();  // Evaluate model logic

    if (top->clk) {
        // 2. Initiate transactions here
        if (cycle_count == 10) {
            std::vector<uint8_t> data = {0xAA, 0xBB, 0xCC, 0xDD};
            mst.write_incr(0x1000, data);
        }

        // 3. Update output signals
        mst.update_output();
        slv.update_output();
    }
}
```

### 5. AXI4-Stream Image VIP Special Features

**Standard Compliance**: The axis_image_vip module follows the **Xilinx AXI4-Stream Video IP and System Design** standard, ensuring compatibility with Xilinx/AMD video processing IP cores. For detailed specifications, refer to the [Xilinx AXI Video IP Product Guide](https://docs.xilinx.com/r/en-US/pg034_axi_video_ip).

**Image Master** (BMP to AXI4-Stream):
```cpp
// Parameters: <BPC (bits per channel), PPC (pixels per beat)>
axis_image_master<8, 4> img_mst(s_mst_port);

// Read BMP and start sending (non-blocking)
img_mst.send_frame("test.bmp");

// In simulation loop
img_mst.update_input();
top->eval();
img_mst.update_output();

// Check if sending is complete
if (img_mst.eof()) {
    // Image transfer done
}
```

**Image Slave** (AXI4-Stream to BMP):
```cpp
axis_image_slave<8, 4> img_slv{m_slv_port};

// Prepare to receive image with known resolution
img_slv.receive_image(1920, 1080);

// In simulation loop
img_slv.update_input();
top->eval();
img_slv.update_output();

// Save received image
if (img_slv.eof()) {
    img_slv.save_bmp("output.bmp");
}
```

## 🔧 Running Tests

### Environment Setup
Ensure you have:
- Verilator (v5.038+)
- GTKWave (optional, for waveform viewing)
- Make
- G++ (C++11 or higher)

### Execute Tests
For basic testing, follow the same commands shown in the **Quick Start** section. Each test generates a `waveform.vcd` file for waveform viewing with GTKWave or other VCD viewers.

## 🙏 Acknowledgements

The design inspiration for this project comes from [soc-simulator](https://github.com/cyyself/soc-simulator). Thanks for its excellent concepts.

## 📄 License

MIT License

Copyright (c) 2025 WanderingKitsune

---

<span id="cn">vaxivip</span>
===========================

**vaxivip** 是一个用于 [Verilator](https://www.veripool.org/verilator/) 仿真的轻量级 AXI 协议验证 IP (VIP) 库。它用 C++ 编写，可以在 C++ 测试平台中模拟 AXI Master 和 Slave 行为，从而验证 Verilog/SystemVerilog 设计。

## ✨ 特性概览

| VIP模块 | 协议 | 核心功能 | 典型应用 |
|---------|------|----------|----------|
| **`axi_vip`** | AXI4 | 内存映射读写、突发传输 | 处理器/内存总线验证 |
| **`axil_vip`** | AXI4-Lite | 简单寄存器访问 | 外设寄存器访问验证 |
| **`axis_vip`** | AXI4-Stream | 流数据传输 | 视频流、网络包传输 |
| **`axis_image_vip`** | AXI4-Stream | BMP图像流，可配置BPC/PPC (遵循Xilinx AXI4-Stream Video标准) | 图像处理IP验证（BMP输入/输出） |

## 🚀 快速开始

### 1. 安装依赖
```bash
# Ubuntu/Debian
sudo apt-get install verilator g++ make

# macOS (使用 Homebrew)
brew install verilator
```

### 2. 克隆项目
```bash
git clone https://github.com/WanderingKitsune/vaxivip.git
cd vaxivip
```

### 3. 编译和运行测试
```bash
# 进入测试目录
cd tb/axi

# 编译并运行仿真
make

# 查看波形（可选）
# gtkwave waveform.vcd

# 清理构建文件
make clean
```

💡 **提示**：查看其他测试目录（`tb/axil/`、`tb/axis/`、`tb/axis_image/`）获取更多示例！

## 📖 详细使用指南

### 项目结构
```
.
├── src/                # VIP源代码
│   ├── axi_vip/        # AXI4 VIP (Master/Slave)
│   ├── axil_vip/       # AXI4-Lite VIP (Master/Slave)
│   ├── axis_vip/       # AXI4-Stream VIP (Master/Slave)
│   └── axis_image_vip/ # AXI4-Stream 图像VIP (支持BMP)
├── tb/                 # 测试用例和示例
│   ├── axi/            # AXI4 测试
│   ├── axil/           # AXI4-Lite 测试
│   ├── axis/           # AXI4-Stream 测试
│   └── axis_image/     # AXI4-Stream 图像测试
└── README.md
```

### 1. 引入头文件
```cpp
#include "axi_vip/axi_vip.hpp"        // AXI4 VIP
#include "axil_vip/axil_vip.hpp"      // AXI4-Lite VIP  
#include "axis_vip/axis_vip.hpp"      // AXI4-Stream VIP
#include "axis_image_vip/axis_image_vip.hpp"  // AXI4-Stream 图像VIP
```

### 2. 绑定信号
将 Verilated 模型的信号指针绑定到 VIP 的信号结构体：
```cpp
// 定义AXI总线结构体（与DUT参数匹配）
axi_ptr<256, 40, 16> axi_sig;  // DataWidth=256, AddrWidth=40, IdWidth=16

// 绑定信号（将VIP指针指向Verilator模型信号）
axi_sig.awaddr  = &(top->m_axi_awaddr);
axi_sig.awvalid = &(top->m_axi_awvalid);
axi_sig.awready = &(top->m_axi_awready);
// ... 绑定所有必需的AXI信号
```

### 3. 创建VIP实例
```cpp
// 创建AXI Master实例
axi_master<256, 40, 16> mst(axi_sig);

// 或创建AXI Slave实例
axi_slave<256, 40, 16> slv(axi_sig);
```

### 4. 在仿真循环中驱动

**重要提示**：在 `eval()` 之前调用 `update_input()`，以正确模拟时钟沿的输入信号采样。

```cpp
while (!Verilated::gotFinish()) {
    // ... 时钟生成 ...
    
    if (top->clk) {
        // 1. 读取输入信号（在时钟沿采样）
        mst.update_input();
        slv.update_input();
    }

    top->eval();  // 计算模型逻辑

    if (top->clk) {
        // 2. 在此处发起事务
        if (cycle_count == 10) {
            std::vector<uint8_t> data = {0xAA, 0xBB, 0xCC, 0xDD};
            mst.write_incr(0x1000, data);
        }

        // 3. 更新输出信号
        mst.update_output();
        slv.update_output();
    }
}
```

### 5. AXI4-Stream 图像VIP特殊功能

**标准遵循**：axis_image_vip 模块遵循 **Xilinx AXI4-Stream Video IP and System Design** 标准，确保与 Xilinx/AMD 视频处理 IP 核的兼容性。详细规范请参考 [Xilinx AXI Video IP 产品指南](https://docs.xilinx.com/r/en-US/pg034_axi_video_ip)。

**图像Master** (BMP转AXI4-Stream):
```cpp
// 参数: <BPC (每通道位数), PPC (每拍像素数)>
axis_image_master<8, 4> img_mst(s_mst_port);

// 读取BMP并开始发送（非阻塞）
img_mst.send_frame("test.bmp");

// 在仿真循环中
img_mst.update_input();
top->eval();
img_mst.update_output();

// 检查发送是否完成
if (img_mst.eof()) {
    // 图像传输完成
}
```

**图像Slave** (AXI4-Stream转BMP):
```cpp
axis_image_slave<8, 4> img_slv{m_slv_port};

// 准备接收已知分辨率的图像
img_slv.receive_image(1920, 1080);

// 在仿真循环中
img_slv.update_input();
top->eval();
img_slv.update_output();

// 保存接收到的图像
if (img_slv.eof()) {
    img_slv.save_bmp("output.bmp");
}
```

## 🔧 运行测试

### 环境搭建
确保已安装：
- Verilator (v5.038+)
- GTKWave (可选，用于查看波形)
- Make
- G++ (支持C++11或更高版本)

### 执行测试
基础测试请参考**快速开始**部分中的命令。每个测试都会生成 `waveform.vcd` 波形文件，可使用 GTKWave 或其他 VCD 查看器查看。

## 🙏 致谢

本项目的设计灵感来源于[soc-simulator](https://github.com/cyyself/soc-simulator)，感谢其优秀的构思。

## 📄 版权说明

MIT License

Copyright (c) 2025 WanderingKitsune