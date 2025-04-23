#!/bin/bash

# 设置颜色代码
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # 无颜色

# 设置工作目录 - 移动到项目根目录
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT" || exit 1

echo -e "${BLUE}清理测试输出文件...${NC}"

# 删除测试输出文件
echo -e "${YELLOW}删除测试失败文件...${NC}"
rm -f failed_test_*.txt
rm -f standard_output_*.txt
rm -f your_output_*.txt

# 删除数据库文件
echo -e "${YELLOW}删除数据库文件...${NC}"
rm -f database.block database.index
rm -f build/database.block build/database.index

# 删除临时目录
echo -e "${YELLOW}删除临时目录...${NC}"
rm -rf temp_test_*

# 删除其他输出文件
echo -e "${YELLOW}删除其他输出文件...${NC}"
rm -f output.txt build/output.txt
rm -f test_data.txt build/test_data.txt

echo -e "${GREEN}清理完成!${NC}"