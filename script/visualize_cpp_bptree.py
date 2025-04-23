#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.animation import FuncAnimation
import argparse
import os
import sys

class BPTreeVisualizer:
    """从C++导出的B+树可视化工具"""
    def __init__(self):
        self.fig = None
        self.ax = None
        self.tree_data = None
        
    def load_tree(self, json_file):
        """从JSON文件加载B+树结构"""
        with open(json_file, 'r') as f:
            self.tree_data = json.load(f)
        return self.tree_data
        
    def calculate_layout(self):
        """计算节点布局"""
        # 按层次组织节点
        levels = {}
        node_dict = {node["id"]: node for node in self.tree_data["nodes"]}
        
        # 构建父子关系
        child_to_parent = {}
        for link in self.tree_data["links"]:
            if link["type"] == "parent-child":
                child_to_parent[link["target"]] = link["source"]
        
        # 计算每个节点的层级
        for node_id in node_dict:
            level = 0
            current = node_id
            while current in child_to_parent:
                level += 1
                current = child_to_parent[current]
            
            if level not in levels:
                levels[level] = []
            levels[level].append(node_id)
        
        # 为每个节点分配位置
        positions = {}
        max_level = max(levels.keys()) if levels else 0
        
        for level in range(max_level + 1):
            nodes = levels.get(level, [])
            width = len(nodes)
            for i, node_id in enumerate(sorted(nodes)):
                # 水平均匀分布，垂直按层级
                x = (i + 0.5) / (width + 0.5) if width > 0 else 0.5
                y = 1 - level / (max_level + 1) if max_level > 0 else 0.5
                positions[node_id] = (x, y)
        
        return positions, node_dict
    
    def render(self, title="B+ Tree Visualization"):
        """渲染B+树可视化"""
        if not self.tree_data:
            raise ValueError("没有加载B+树数据")
            
        self.fig, self.ax = plt.subplots(figsize=(12, 8))
        
        # 设置边界
        self.ax.set_xlim(0, 1)
        self.ax.set_ylim(0, 1)
        self.ax.axis('off')
        
        # 显示标题
        self.ax.set_title(title, fontsize=14)
        
        positions, node_dict = self.calculate_layout()
        
        # 绘制节点
        for node in self.tree_data["nodes"]:
            node_id = node["id"]
            x, y = positions[node_id]
            
            # 节点宽度与键数量相关
            width = max(0.1, min(0.3, 0.05 * (len(node["keys"]) + 1)))
            height = 0.1
            
            # 绘制节点框
            rect = patches.Rectangle(
                (x - width/2, y - height/2), 
                width, height, 
                linewidth=1, 
                edgecolor='black', 
                facecolor='lightblue' if node["is_leaf"] else 'lightgreen',
                alpha=0.7
            )
            self.ax.add_patch(rect)
            
            # 绘制节点ID
            self.ax.text(x, y + height/1.5, f"Node {node_id}", 
                        ha='center', va='center', fontsize=8)
            
            # 绘制键
            keys_text = ", ".join(map(str, node["keys"]))
            self.ax.text(x, y, keys_text, ha='center', va='center', fontsize=10)
            
            # 绘制值（如果是叶子节点）
            if node["is_leaf"]:
                values_text = ""
                for i, values in enumerate(node.get("values", [])):
                    if i > 0:
                        values_text += "; "
                    values_text += f"{node['keys'][i]}→{','.join(map(str, values))}"
                self.ax.text(x, y - height/1.5, values_text, 
                            ha='center', va='center', fontsize=8)
        
        # 绘制连接线
        for link in self.tree_data["links"]:
            source_id = link["source"]
            target_id = link["target"]
            
            if source_id in positions and target_id in positions:
                x1, y1 = positions[source_id]
                x2, y2 = positions[target_id]
                
                if link["type"] == "parent-child":
                    # 父子连接是实线
                    self.ax.plot([x1, x2], [y1, y2], 'b-', alpha=0.6, zorder=1)
                elif link["type"] == "leaf-link":
                    # 叶子节点链接是虚线
                    self.ax.plot([x1, x2], [y1, y2], 'r--', alpha=0.6, zorder=1)
        
        plt.tight_layout()
        
    def save(self, filename='bptree_visualization.png'):
        """保存可视化到文件"""
        if not self.fig:
            self.render()
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        print(f"可视化已保存到: {filename}")
        
    def show(self):
        """显示可视化"""
        if not self.fig:
            self.render()
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='可视化C++实现的B+树')
    parser.add_argument('-i', '--input', required=True, help='包含B+树结构的JSON文件')
    parser.add_argument('-o', '--output', help='输出图片文件名')
    parser.add_argument('-t', '--title', default='B+ Tree Visualization', help='可视化标题')
    parser.add_argument('--show', action='store_true', help='显示可视化窗口')
    args = parser.parse_args()
    
    # 检查输入文件
    if not os.path.exists(args.input):
        print(f"错误: 输入文件 {args.input} 不存在")
        sys.exit(1)
    
    # 创建可视化器
    visualizer = BPTreeVisualizer()
    
    # 加载树结构
    try:
        visualizer.load_tree(args.input)
    except Exception as e:
        print(f"加载B+树结构失败: {e}")
        sys.exit(1)
    
    # 渲染可视化
    try:
        visualizer.render(args.title)
    except Exception as e:
        print(f"渲染B+树失败: {e}")
        sys.exit(1)
    
    # 保存或显示
    if args.output:
        visualizer.save(args.output)
    
    if args.show:
        visualizer.show()
    elif not args.output:
        # 如果既没有指定保存也没有指定显示，默认显示
        visualizer.show()

if __name__ == "__main__":
    main()