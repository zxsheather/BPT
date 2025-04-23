#!/bin/bash
# 文件名: test_bptree.sh

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

# 设置测试参数的默认值
TEST_COUNT=5           # 测试组数
OPERATION_COUNT=1000   # 每组测试的操作数
INDEX_RANGE=200        # 索引范围
VALUE_RANGE=5000       # 值范围
INSERT_RATIO=0.6       # 插入操作比例
DELETE_RATIO=0.2       # 删除操作比例
FIND_RATIO=0.2         # 查找操作比例

# 显示帮助信息
show_help() {
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -h, --help           显示帮助信息"
    echo "  -t, --tests NUMBER   设置测试组数 (默认: $TEST_COUNT)"
    echo "  -o, --ops NUMBER     设置每组测试的操作数 (默认: $OPERATION_COUNT)"
    echo "  -i, --index NUMBER   设置索引范围 (默认: $INDEX_RANGE)"
    echo "  -v, --value NUMBER   设置值范围 (默认: $VALUE_RANGE)"
    echo "  --insert RATIO       设置插入操作比例 (默认: $INSERT_RATIO)"
    echo "  --delete RATIO       设置删除操作比例 (默认: $DELETE_RATIO)"
    echo "  --find RATIO         设置查找操作比例 (默认: $FIND_RATIO)"
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -t|--tests)
            TEST_COUNT="$2"
            shift 2
            ;;
        -o|--ops)
            OPERATION_COUNT="$2"
            shift 2
            ;;
        -i|--index)
            INDEX_RANGE="$2"
            shift 2
            ;;
        -v|--value)
            VALUE_RANGE="$2"
            shift 2
            ;;
        --insert)
            INSERT_RATIO="$2"
            shift 2
            ;;
        --delete)
            DELETE_RATIO="$2"
            shift 2
            ;;
        --find)
            FIND_RATIO="$2"
            shift 2
            ;;
        *)
            echo "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
done

# 检查必要文件是否存在
if [ ! -f "script/data_bptree_tests.py" ]; then
    echo -e "${RED}错误: script/data_bptree_tests.py 不存在${NC}"
    exit 1
fi

if [ ! -f "script/bptree_standard.py" ]; then
    echo -e "${RED}错误: script/bptree_standard.py 不存在${NC}"
    exit 1
fi

# 确保构建目录存在
if [ ! -d "build" ]; then
    mkdir -p build
fi

# 构建C++程序
echo -e "${BLUE}构建C++程序...${NC}"
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
cd ..

# 检查构建是否成功
if [ ! -f "build/bpt_main" ]; then
    echo -e "${RED}错误: 构建失败，找不到可执行文件 build/bpt_main${NC}"
    exit 1
fi

# 创建临时目录
TEMP_DIR="temp_test_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$TEMP_DIR"

echo -e "${BLUE}开始测试...${NC}"
echo -e "${YELLOW}测试参数:${NC}"
echo "  测试组数: $TEST_COUNT"
echo "  每组操作数: $OPERATION_COUNT"
echo "  索引范围: $INDEX_RANGE"
echo "  值范围: $VALUE_RANGE"
echo "  插入比例: $INSERT_RATIO"
echo "  删除比例: $DELETE_RATIO"
echo "  查找比例: $FIND_RATIO"

PASSED=0
FAILED=0

# 运行测试
for ((i=1; i<=TEST_COUNT; i++)); do
    echo -e "${YELLOW}运行测试 $i/$TEST_COUNT...${NC}"
    
    # 生成随机种子
    SEED=$RANDOM
    
    # 生成测试数据
    echo "生成测试数据 (种子: $SEED)..."
    python3 script/data_bptree_tests.py \
        --count "$OPERATION_COUNT" \
        --index-range "$INDEX_RANGE" \
        --value-range "$VALUE_RANGE" \
        --insert-ratio "$INSERT_RATIO" \
        --delete-ratio "$DELETE_RATIO" \
        --find-ratio "$FIND_RATIO" \
        --seed "$SEED" \
        --output "$TEMP_DIR/test_$i.txt"
    
    # 运行Python标准程序
    echo "运行Python标准程序..."
    cat "$TEMP_DIR/test_$i.txt" | python3 script/bptree_standard.py > "$TEMP_DIR/standard_$i.txt"
    
    # 运行C++程序
    echo "运行C++程序..."
    cd build
    rm -f database.block database.index  # 清理之前的数据文件
    cat "../$TEMP_DIR/test_$i.txt" | ./bpt_main > "../$TEMP_DIR/output_$i.txt" 2> "../$TEMP_DIR/error_$i.txt"
    cd ..
    
    # 检查错误输出
    if [ -s "$TEMP_DIR/error_$i.txt" ]; then
        echo -e "${RED}C++程序运行出错:${NC}"
        cat "$TEMP_DIR/error_$i.txt"
    fi
    
    # 检查输出文件是否为空
    if [ ! -s "$TEMP_DIR/output_$i.txt" ]; then
        echo -e "${RED}警告: C++程序没有输出任何内容${NC}"
        # 为了调试目的，让我们看看输入内容
        echo -e "${YELLOW}输入内容前20行:${NC}"
        head -n 20 "$TEMP_DIR/test_$i.txt"
    fi
    
    # 比较输出 (忽略空白差异)
    echo "比较输出结果 (忽略空白差异)..."
    if diff -qwB "$TEMP_DIR/standard_$i.txt" "$TEMP_DIR/output_$i.txt" > /dev/null; then
        echo -e "${GREEN}测试 $i 通过!${NC}"
        PASSED=$((PASSED+1))
    else
        echo -e "${RED}测试 $i 失败!${NC}"
        echo "差异如下 (忽略空白差异):"
        diff -wB "$TEMP_DIR/standard_$i.txt" "$TEMP_DIR/output_$i.txt" | head -n 10
        
        # 保存失败的测试用例
        cp "$TEMP_DIR/test_$i.txt" "failed_test_$i.txt"
        cp "$TEMP_DIR/standard_$i.txt" "standard_output_$i.txt"
        cp "$TEMP_DIR/output_$i.txt" "your_output_$i.txt"
        echo "保存失败的测试用例到: failed_test_$i.txt"
        echo "保存标准输出到: standard_output_$i.txt"
        echo "保存你的输出到: your_output_$i.txt"
        
        FAILED=$((FAILED+1))
    fi
    
    echo "----------------------------------------"
done

# 显示测试结果
echo -e "${YELLOW}测试总结:${NC}"
echo -e "  ${GREEN}通过: $PASSED${NC}"
echo -e "  ${RED}失败: $FAILED${NC}"

# 显示最终结果
if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}所有测试通过!${NC}"
else
    echo -e "${RED}有 $FAILED 个测试失败!${NC}"
fi

# 清理临时文件
read -p "是否删除临时文件? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "$TEMP_DIR"
    echo "已删除临时文件"
fi

exit $FAILED