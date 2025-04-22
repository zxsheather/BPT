
def process_operations(operations):
    # 使用字典存储索引到值的映射，一个索引可以对应多个值
    data = {}
    results = []
    
    for op in operations:
        parts = op.strip().split()
        command = parts[0]
        
        if command == "insert":
            index, value = int(parts[1]), int(parts[2])
            if index not in data:
                data[index] = []
            data[index].append(value)
        
        elif command == "delete":
            index, value = int(parts[1]), int(parts[2])
            if index in data and value in data[index]:
                data[index].remove(value)
                # 如果列表为空，删除该索引
                if not data[index]:
                    del data[index]
        
        elif command == "find":
            index = int(parts[1])
            if index in data and data[index]:
                # 按值升序排序并输出
                values = sorted(data[index])
                results.append(" ".join(map(str, values)))
            else:
                results.append("null")
    
    return results

def main():
    n = int(input().strip())
    operations = []
    
    for _ in range(n):
        operations.append(input().strip())
    
    results = process_operations(operations)
    for result in results:
        print(result)

if __name__ == "__main__":
    main()