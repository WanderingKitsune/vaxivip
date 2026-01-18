# vaxivip

![Language](https://img.shields.io/badge/Language-C++-00599C.svg) ![Simulation](https://img.shields.io/badge/Simulation-Verilator-green.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg)

[English](#en) | [中文](#cn)



<span id="en">vaxivip</span>
===========================

vaxivip is a lightweight AXI protocol Verification IP (VIP) library for [Verilator](https://www.veripool.org/verilator/) simulation. Written in C++, it simulates AXI Master and Slave behaviors in C++ testbenches to verify Verilog/SystemVerilog designs.

## Directory Structure

```
.
├── src/                # VIP source code, signal definitions, and logging
│   ├── axi_vip/        # AXI4 VIP (Master/Slave)
│   ├── axil_vip/       # AXI4-Lite VIP (Master/Slave)
│   └── axis_vip/       # AXI4-Stream VIP (Master/Slave)
├── tb/                 # Testbench
│   ├── axi/            # AXI4 test code
│   ├── axil/           # AXI4-Lite test code
│   └── axis/           # AXI4-Stream test code
└── README.md
```

## Usage

### 1. Include Headers

Include the corresponding header files in your C++ testbench (`.cpp`):

```cpp
#include "axi_vip/axi_vip.hpp"
// Or
// #include "axil_vip/axil_vip.hpp"
// #include "axis_vip/axis_vip.hpp"
```

### 2. Bind Signals

Bind Verilated model signal pointers to the VIP signal structure.

```cpp
// Top-level module instance
Vtop* top = new Vtop;

// 1. Define AXI bus structure. Note: Data/Addr widths must match the top level.
axi_ptr<256, 40, 16> axi_sig; // DataWidth=256, AddrWidth=40, IdWidth=16

// 2. Bind signals (Point VIP signal pointers to Verilator model signals)
// Example: Bind Master interface
axi_sig.awaddr  = &(top->m_axi_awaddr);
axi_sig.awvalid = &(top->m_axi_awvalid);
axi_sig.awready = &(top->m_axi_awready);
// ... Bind all required AXI signals ...
```

### 3. Instantiate VIP

Instantiate Master or Slave objects:

```cpp
// Instantiate an AXI Master VIP
// Parameters: <DataWidth, AddrWidth, IdWidth>
axi_master<256, 40, 16> mst(axi_sig);

// Or instantiate a Slave VIP
axi_slave<256, 40, 16> slv(axi_sig);
```

### 4. Drive in Simulation Loop

Note: Call `update_input()` before `eval()` to simulate sampling input signals at clock edges.

```cpp
while (!Verilated::gotFinish()) {
    // ... Clock generation ...
    
    if (top->clk) {
        // 1. Read input signals
        mst.update_input();
        slv.update_input();
    }

    top->eval(); // Evaluate model logic

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

## Run Tests

### 1. Environment Setup

Please ensure the following tools are installed:
- Verilator (v5.038+)
- GTKWave (Optional, for waveform viewing)
- Make
- G++ (Support C++11 or higher)

### 2. Execute Tests

Enter the test directory and run `make` to compile and execute the simulation. By default, the simulation generates a `waveform.vcd` file.

Taking AXI4 interface test as an example:

```bash
cd tb/axi
make        # Compile and run simulation
```

### 3. Clean Build

```bash
make clean
```

## Acknowledgements

The design inspiration for this project comes from [soc-simulator](https://github.com/cyyself/soc-simulator). Thanks for its excellent concepts.

## License

MIT License

Copyright (c) 2025 WanderingKitsune

　

　

<span id="cn">vaxivip</span>
===========================

vaxivip 是一个用于 [Verilator](https://www.veripool.org/verilator/) 仿真的轻量级 AXI 协议验证 IP (VIP) 库。它用 C++ 编写，可以在 C++ 测试平台中模拟 AXI Master 和 Slave 行为，从而验证 Verilog/SystemVerilog 设计。

## 目录结构

```
.
├── src/                # VIP源代码，信号定义和日志
│   ├── axi_vip/        # AXI4 VIP (Master/Slave)
│   ├── axil_vip/       # AXI4-Lite VIP (Master/Slave)
│   └── axis_vip/       # AXI4-Stream VIP (Master/Slave)
├── tb/                 # 测试用例 (Testbench)
│   ├── axi/            # AXI4 测试代码
│   ├── axil/           # AXI4-Lite 测试代码
│   └── axis/           # AXI4-Stream 测试代码
└── README.md
```

## 使用方法

### 1. 引入头文件

在 C++ 测试平台 (`.cpp`) 中包含相应的头文件：

```cpp
#include "axi_vip/axi_vip.hpp"
// 或者
// #include "axil_vip/axil_vip.hpp"
// #include "axis_vip/axis_vip.hpp"
```

### 2. 绑定信号

将 Verilated 模型的信号指针绑定到 VIP 的信号结构体中。

```cpp
// 顶层模块实例
Vtop* top = new Vtop;

// 1. 定义axi总线结构体，注意型号位宽设置要和顶层一致
axi_ptr<256, 40, 16> axi_sig; // DataWidth=256, AddrWidth=40, IdWidth=16

// 2. 绑定信号 (将 VIP 的信号指针指向 Verilator 模型的信号)
// 例如，绑定 Master 接口
axi_sig.awaddr  = &(top->m_axi_awaddr);
axi_sig.awvalid = &(top->m_axi_awvalid);
axi_sig.awready = &(top->m_axi_awready);
// ... 绑定所有必需的 AXI 信号 ...
```

### 3. 实例化 VIP

实例化 Master 或 Slave 对象：

```cpp
// 实例化一个 AXI Master VIP
// 参数: <DataWidth, AddrWidth, IdWidth>
axi_master<256, 40, 16> mst(axi_sig);

// 或者实例化一个 Slave VIP
axi_slave<256, 40, 16> slv(axi_sig);
```

### 4. 在仿真循环中驱动

注意调用update_input()函数要在eval()前调用，以此来模拟时钟沿采样输入信号。
```cpp
while (!Verilated::gotFinish()) {
    // ... 时钟生成 ...
    
    if (top->clk) {
        // 1. 读取输入信号
        mst.update_input();
        slv.update_input();
    }

    top->eval(); // 计算模型逻辑

    if (top->clk) {
        // 2. 可以在这里发起突发
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

## 运行测试

### 1. 环境搭建

请确保已安装以下工具：
- Verilator (v5.038+)
- GTKWave (可选，用于查看波形)
- Make
- G++ (支持 C++11 或更高版本)

### 2. 执行测试

进入测试目录并运行 `make` 即可编译并执行仿真。默认情况下，仿真会生成 `waveform.vcd` 波形文件。

以 AXI4 接口测试为例：

```bash
cd tb/axi
make        # 编译并运行仿真
```

### 3. 清理构建

```bash
make clean
```

## 致谢

本项目的设计灵感来源于[soc-simulator](https://github.com/cyyself/soc-simulator)，感谢其优秀的构思。

## 版权说明

MIT License

Copyright (c) 2025 WanderingKitsune
