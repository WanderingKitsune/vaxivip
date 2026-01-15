# vaxivip

vaxivip 是一个用于 [Verilator](https://www.veripool.org/verilator/) 仿真的轻量级 AXI 协议验证 IP (VIP) 库。它用 C++ 编写，可以在 C++ 测试平台中模拟 AXI Master 和 Slave 行为，从而验证 Verilog/SystemVerilog 设计。

## 目录结构

```
.
├── src/                # VIP源代码，信号定义和日志
│   ├── axi_vip/        # AXI4 VIP 实现 (Master/Slave)
│   ├── axil_vip/       # AXI4-Lite VIP 实现 (Master/Slave)
│   └── axis_vip/       # AXI4-Stream VIP 实现 (Master/Slave)
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

// 1. 定义axi总线结构体
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

注意调用update_input()函数要在eval()前调用，以此来模拟时钟沿采样输入信号
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
## 致谢

感谢https://github.com/cyyself/soc-simulator项目给我的启发

## 版权说明

MIT License

Copyright (c) 2025 WanderingKitsune
