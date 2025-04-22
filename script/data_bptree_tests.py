import random
import argparse

def generate_test_data(operation_count=1000, index_range=1000, value_range=10000, 
                      insert_ratio=0.6, delete_ratio=0.2, find_ratio=0.2,
                      seed=None, output_file=None):
    """
    生成B+树测试数据
    
    参数:
    - operation_count: 总操作数
    - index_range: 索引范围 (0到index_range-1)
    - value_range: 值范围 (0到value_range-1)
    - insert_ratio: 插入操作的比例
    - delete_ratio: 删除操作的比例
    - find_ratio: 查找操作的比例
    - seed: 随机数种子
    - output_file: 输出文件路径，如果为None则输出到控制台
    """
    if seed is not None:
        random.seed(seed)
    
    # 验证比例总和是否为1
    total_ratio = insert_ratio + delete_ratio + find_ratio
    if abs(total_ratio - 1.0) > 1e-6:
        print("警告: 操作比例总和不为1，将进行自动归一化")
        insert_ratio /= total_ratio
        delete_ratio /= total_ratio
        find_ratio /= total_ratio
    
    # 计算每种操作的数量
    insert_count = int(operation_count * insert_ratio)
    delete_count = int(operation_count * delete_ratio)
    find_count = operation_count - insert_count - delete_count
    
    operations = []
    
    # 记录每个索引的值，用于确保相同索引下有不同值
    index_value_map = {}
    
    # 保证部分索引有多个不同的值（约20%的索引）
    multi_value_indices_count = min(int(index_range * 0.2), insert_count // 3)
    multi_value_indices = random.sample(range(index_range), multi_value_indices_count)
    
    # 为这些特殊索引分配多个值（每个索引至少2个不同值）
    values_per_index = max(2, min(5, insert_count // (multi_value_indices_count * 2)))
    multi_value_insert_count = multi_value_indices_count * values_per_index
    
    # 先生成这部分特殊的插入操作
    for idx in multi_value_indices:
        index_value_map[idx] = set()
        # 为每个特殊索引生成多个不同的值
        for _ in range(values_per_index):
            while True:
                value = random.randint(0, value_range-1)
                if value not in index_value_map[idx]:
                    index_value_map[idx].add(value)
                    operations.append(f"insert {idx} {value}")
                    break
    
    # 生成剩余的随机插入操作
    remaining_insert_count = insert_count - multi_value_insert_count
    for _ in range(remaining_insert_count):
        index = random.randint(0, index_range-1)
        # 如果这个索引已经有值，确保生成一个不同的值
        if index in index_value_map:
            while True:
                value = random.randint(0, value_range-1)
                if value not in index_value_map[index]:
                    index_value_map[index].add(value)
                    operations.append(f"insert {index} {value}")
                    break
        else:
            value = random.randint(0, value_range-1)
            index_value_map[index] = {value}
            operations.append(f"insert {index} {value}")
    
    # 生成删除操作
    for _ in range(delete_count):
        # 有一半的删除操作针对已存在的条目
        if random.random() < 0.5 and index_value_map:
            index = random.choice(list(index_value_map.keys()))
            # 如果该索引有多个值，随机选择一个删除
            if index_value_map[index]:
                value = random.choice(list(index_value_map[index]))
                operations.append(f"delete {index} {value}")
                index_value_map[index].remove(value)
                if not index_value_map[index]:
                    del index_value_map[index]
            else:
                # 生成随机删除
                index = random.randint(0, index_range-1)
                value = random.randint(0, value_range-1)
                operations.append(f"delete {index} {value}")
        else:
            # 生成随机删除
            index = random.randint(0, index_range-1)
            value = random.randint(0, value_range-1)
            operations.append(f"delete {index} {value}")
    
    # 生成查找操作
    for _ in range(find_count):
        # 有一半的查找操作针对已存在的索引
        if random.random() < 0.5 and index_value_map:
            index = random.choice(list(index_value_map.keys()))
        else:
            index = random.randint(0, index_range-1)
        operations.append(f"find {index}")
    
    # 随机打乱操作顺序
    random.shuffle(operations)
    
    # 输出结果
    if output_file:
        with open(output_file, 'w') as f:
            f.write(f"{len(operations)}\n")
            for op in operations:
                f.write(f"{op}\n")
        print(f"已生成 {len(operations)} 条测试数据，保存至 {output_file}")
    else:
        print(f"{len(operations)}")
        for op in operations:
            print(op)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="生成B+树测试数据")
    parser.add_argument("--count", type=int, default=1000, help="总操作数")
    parser.add_argument("--index-range", type=int, default=1000, help="索引范围")
    parser.add_argument("--value-range", type=int, default=10000, help="值范围")
    parser.add_argument("--insert-ratio", type=float, default=0.6, help="插入操作比例")
    parser.add_argument("--delete-ratio", type=float, default=0.2, help="删除操作比例")
    parser.add_argument("--find-ratio", type=float, default=0.2, help="查找操作比例")
    parser.add_argument("--seed", type=int, default=None, help="随机数种子")
    parser.add_argument("--output", type=str, default=None, help="输出文件路径")
    
    args = parser.parse_args()
    
    generate_test_data(
        operation_count=args.count,
        index_range=args.index_range,
        value_range=args.value_range,
        insert_ratio=args.insert_ratio,
        delete_ratio=args.delete_ratio,
        find_ratio=args.find_ratio,
        seed=args.seed,
        output_file=args.output
    )