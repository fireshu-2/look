# SSNE AI Face Detection Demo

## 项目概述

这是一个基于 **SmartSens SSNE (SmartSens Neural Engine)** 的 **YOLOv8 人脸检测**演示程序。

## 功能特性

- ✅ 实时人脸检测
- ✅ OSD 可视化（检测框绘制）
- ✅ 可调节置信度阈值
- ✅ 支持多个人脸同时检测
- ✅ NMS 非极大值抑制

## 文件结构

```
ssne_ai_demo/
├── demo_face_detection.cpp     # 主演示程序（带UI控制）
├── include/                     # 头文件
│   ├── common.hpp              # IMAGEPROCESSOR、FACE_DETECTOR 类
│   ├── utils.hpp               # VISUALIZER 可视化器类
│   ├── osd-device.hpp          # OSD 设备接口
│   └── log.hpp                 # 日志定义
├── src/                        # 源代码
│   ├── face_detector.cpp       # YOLOv8 检测模型实现
│   ├── pipeline_image.cpp       # 图像处理管道
│   ├── utils.cpp                # 可视化工具实现
│   └── osd-device.cpp           # OSD 设备驱动
├── app_assets/                 # 资源文件
│   ├── models/                 # AI 模型
│   │   └── yolov8n.m1model     # YOLOv8n 人脸检测模型
│   └── bitmaps/                # OSD 位图资源
│       └── shared_colorLUT.sscl # 颜色查找表
├── cmake_config/               # CMake 配置
│   └── Paths.cmake            # SDK 路径配置
└── CMakeLists.txt             # 构建配置
```

## 构建说明

### 1. 配置环境变量

```bash
export M1_SDK_INC_DIR=/path/to/sdk/include
export M1_SDK_LIB_DIR=/path/to/sdk/lib
```

### 2. 创建构建目录

```bash
mkdir build && cd build
```

### 3. CMake 配置

```bash
cmake .. -DBASE_DIR=/path/to/target
```

### 4. 编译

```bash
make -j4
```

### 5. 安装

```bash
make install
```

## 运行说明

### 按键控制

| 按键 | 功能 |
|------|------|
| `s` / `S` | 开始/暂停检测 |
| `q` / `Q` | 退出程序 |
| `t` / `T` | 调整置信度阈值 |
| `b` / `B` | 显示/隐藏检测框 |
| `c` / `C` | 清空 OSD 图层 |

## 配置参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| 原图尺寸 | 1920×1080 | 输入图像分辨率 |
| 模型输入 | 640×640 | YOLOv8 输入尺寸 |
| 置信度阈值 | 0.5 | 检测结果过滤阈值 |
| NMS 阈值 | 0.45 | 非极大值抑制阈值 |

## 技术架构

### 数据流

```
摄像头 → IMAGEPROCESSOR → FACE_DETECTOR → VISUALIZER → OSD屏幕
         (YUV422_16)     (YOLOv8n)       (绘制检测框)
```

### OSD 图层

| Layer | 类型 | 用途 |
|-------|------|------|
| 0 | QUADRANGLE | 人脸检测框 |
| 1 | QUADRANGLE | 固定图形 |
| 2-4 | RLE/IMAGE | 位图资源 |

## 注意事项

1. 确保模型文件 `yolov8n.m1model` 存在于 `app_assets/models/` 目录
2. 确保颜色 LUT 文件 `shared_colorLUT.sscl` 存在于 `app_assets/` 目录
3. 根据实际摄像头参数调整图像尺寸配置

## License

Copyright (c) 2026 SmartSens