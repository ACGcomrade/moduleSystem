#!/bin/bash

# Module System - Xcode项目生成脚本
# 用于在macOS上通过CMake生成Xcode项目

set -e

echo "==================================="
echo "Module System - Xcode Generator"
echo "==================================="

# 设置Qt路径
export Qt6_DIR="/opt/homebrew/opt/qt@6"
export CMAKE_PREFIX_PATH="/opt/homebrew:$Qt6_DIR"

# 清理旧的构建
if [ -d "build_xcode" ]; then
    echo "清理旧的构建目录..."
    rm -rf build_xcode
fi

# 创建构建目录
mkdir -p build_xcode
cd build_xcode

echo ""
echo "生成Xcode项目..."
/opt/homebrew/bin/cmake -G "Xcode" \
    -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DQt6_DIR="$Qt6_DIR/lib/cmake/Qt6" \
    ..

echo ""
echo "==================================="
echo "✅ Xcode项目生成成功！"
echo "==================================="
echo ""
echo "打开方式："
echo "  open ModuleSystem.xcodeproj"
echo ""
echo "或者在Xcode中："
echo "  File > Open > 选择 build_xcode/ModuleSystem.xcodeproj"
echo ""
echo "注意："
echo "  - 在Xcode中选择 ModuleSystem target"
echo "  - 选择 Product > Scheme > Edit Scheme"
echo "  - 设置 Run > Options > Working Directory 为项目根目录"
echo ""
