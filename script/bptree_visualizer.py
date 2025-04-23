#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np
from matplotlib.animation import FuncAnimation
import sys
import os
import argparse
from collections import deque
import json

class BPTreeNode:
    """B+树节点类"""
    def __init__(self, is_leaf=True, parent=None):
        self.keys = []
        self.values = []  # 仅叶子节点存储值
        self.children = []  # 非叶子节点存储子节点
        self.next = None  # 叶子节点链表指针
        self.is_leaf = is_leaf
        self.parent = parent
        self.node_id = None  # 用于绘图标识

class BPTree:
    """B+树实现"""
    def __init__(self, order=3):
        self.root = BPTreeNode()
        self.order = order  # B+树的阶
        self.node_count = 0  # 节点计数，用于分配ID
        self.root.node_id = self.get_next_id()
        self.history = []  # 用于存储树的状态历史
        self.operations = []  # 对应的操作历史
        self.snapshot()  # 记录初始状态
    
    def get_next_id(self):
        """获取下一个节点ID"""
        self.node_count += 1
        return self.node_count
    
    def snapshot(self, operation="初始状态"):
        """保存当前树状态的快照"""
        # 深拷贝树结构进行存储
        state = self.serialize_tree()
        self.history.append(state)
        self.operations.append(operation)
    
    def serialize_tree(self):
        """将树序列化为可JSON化的结构"""
        if not self.root:
            return {"nodes": [], "links": []}
        
        nodes = []
        links = []
        queue = deque([self.root])
        visited = set()
        
        while queue:
            node = queue.popleft()
            if node.node_id in visited:
                continue
            
            visited.add(node.node_id)
            
            # 添加节点
            node_data = {
                "id": node.node_id,
                "keys": node.keys.copy(),
                "is_leaf": node.is_leaf
            }
            
            if node.is_leaf:
                node_data["values"] = node.values.copy()
            
            nodes.append(node_data)
            
            # 添加子节点链接
            if not node.is_leaf:
                for i, child in enumerate(node.children):
                    if child:
                        links.append({
                            "source": node.node_id,
                            "target": child.node_id,
                            "type": "parent-child"
                        })
                        queue.append(child)
            
            # 添加叶子节点链表链接
            if node.is_leaf and node.next:
                links.append({
                    "source": node.node_id,
                    "target": node.next.node_id,
                    "type": "leaf-link"
                })
        
        return {"nodes": nodes, "links": links}
    
    def insert(self, key, value):
        """在B+树中插入键值对"""
        operation = f"插入 ({key}:{value})"
        
        # 如果根为空，直接在根节点插入
        if not self.root.keys:
            self.root.keys.append(key)
            self.root.values.append([value])
            self.snapshot(operation)
            return
        
        # 查找插入位置
        leaf = self._find_leaf(self.root, key)
        
        # 在叶子节点中找到key的位置
        i = 0
        while i < len(leaf.keys) and key > leaf.keys[i]:
            i += 1
        
        # 如果键已存在，添加值到列表
        if i < len(leaf.keys) and key == leaf.keys[i]:
            if value not in leaf.values[i]:
                leaf.values[i].append(value)
                leaf.values[i].sort()  # 确保值是有序的
                self.snapshot(operation)
            return
        
        # 新键值对，插入到正确位置
        leaf.keys.insert(i, key)
        leaf.values.insert(i, [value])
        
        # 如果节点溢出，需要分裂
        if len(leaf.keys) >= self.order:
            self._split_leaf(leaf)
        
        self.snapshot(operation)
    
    def delete(self, key, value):
        """在B+树中删除键值对"""
        operation = f"删除 ({key}:{value})"
        
        # 如果树为空，直接返回
        if not self.root.keys:
            self.snapshot(operation)
            return
        
        # 查找包含键的叶子节点
        leaf = self._find_leaf(self.root, key)
        
        # 在叶子节点中查找键的位置
        found = False
        for i, k in enumerate(leaf.keys):
            if k == key:
                # 如果找到键，检查值是否存在
                if value in leaf.values[i]:
                    leaf.values[i].remove(value)
                    found = True
                    
                    # 如果值列表为空，删除键
                    if not leaf.values[i]:
                        leaf.keys.pop(i)
                        leaf.values.pop(i)
                    
                    # 处理下溢状态
                    if len(leaf.keys) < (self.order + 1) // 2 and leaf != self.root:
                        self._handle_underflow(leaf)
                    break
        
        # 检查根节点是否为空
        if not self.root.keys and self.root.children:
            self.root = self.root.children[0]
            self.root.parent = None
        
        self.snapshot(operation)
    
    def find(self, key):
        """查找键对应的所有值"""
        operation = f"查询 ({key})"
        
        # 如果树为空，返回空列表
        if not self.root.keys:
            self.snapshot(operation)
            return []
        
        # 查找包含键的叶子节点
        leaf = self._find_leaf(self.root, key)
        
        # 在叶子节点中查找键
        for i, k in enumerate(leaf.keys):
            if k == key:
                self.snapshot(operation)
                return sorted(leaf.values[i])
        
        self.snapshot(operation)
        return []
    
    def _find_leaf(self, node, key):
        """找到应包含键的叶子节点"""
        if node.is_leaf:
            return node
        
        # 在内部节点中查找下一个要访问的子节点
        i = 0
        while i < len(node.keys) and key >= node.keys[i]:
            i += 1
        
        return self._find_leaf(node.children[i], key)
    
    def _split_leaf(self, leaf):
        """分裂叶子节点"""
        # 创建新的叶子节点
        new_leaf = BPTreeNode(is_leaf=True, parent=leaf.parent)
        new_leaf.node_id = self.get_next_id()
        
        # 计算分裂点
        mid = len(leaf.keys) // 2
        
        # 分配键和值
        new_leaf.keys = leaf.keys[mid:]
        new_leaf.values = leaf.values[mid:]
        leaf.keys = leaf.keys[:mid]
        leaf.values = leaf.values[:mid]
        
        # 更新叶子节点链表
        new_leaf.next = leaf.next
        leaf.next = new_leaf
        
        # 如果分裂的是根节点，创建新的根
        if leaf == self.root:
            new_root = BPTreeNode(is_leaf=False)
            new_root.node_id = self.get_next_id()
            new_root.keys = [new_leaf.keys[0]]
            new_root.children = [leaf, new_leaf]
            self.root = new_root
            leaf.parent = new_root
            new_leaf.parent = new_root
        else:
            # 否则，将新节点的第一个键插入父节点
            self._insert_in_parent(leaf, new_leaf.keys[0], new_leaf)
    
    def _insert_in_parent(self, left, key, right):
        """在父节点中插入键和右子节点"""
        parent = left.parent
        
        # 在父节点中找到左子节点的位置
        i = parent.children.index(left)
        
        # 插入键和右子节点
        parent.keys.insert(i, key)
        parent.children.insert(i + 1, right)
        right.parent = parent
        
        # 如果父节点溢出，需要分裂
        if len(parent.keys) >= self.order:
            self._split_internal(parent)
    
    def _split_internal(self, node):
        """分裂内部节点"""
        # 创建新的内部节点
        new_node = BPTreeNode(is_leaf=False, parent=node.parent)
        new_node.node_id = self.get_next_id()
        
        # 计算分裂点
        mid = len(node.keys) // 2
        
        # 上移的键
        promote_key = node.keys[mid]
        
        # 分配键和子节点
        new_node.keys = node.keys[mid+1:]
        new_node.children = node.children[mid+1:]
        node.keys = node.keys[:mid]
        node.children = node.children[:mid+1]
        
        # 更新子节点的父指针
        for child in new_node.children:
            child.parent = new_node
        
        # 如果分裂的是根节点，创建新的根
        if node == self.root:
            new_root = BPTreeNode(is_leaf=False)
            new_root.node_id = self.get_next_id()
            new_root.keys = [promote_key]
            new_root.children = [node, new_node]
            self.root = new_root
            node.parent = new_root
            new_node.parent = new_root
        else:
            # 否则，将提升键插入父节点
            self._insert_in_parent(node, promote_key, new_node)
    
    def _handle_underflow(self, node):
        """处理节点下溢的情况"""
        # 目前简化处理，真实B+树需要处理节点借用和合并
        # 该函数可以根据需要进一步完善
        pass

class BPTreeVisualizer:
    """B+树可视化工具"""
    def __init__(self, tree):
        self.tree = tree
        self.fig = None
        self.ax = None
        self.node_positions = {}
        self.node_patches = {}
        
    def calculate_layout(self, tree_state):
        """计算节点布局"""
        # 按层次组织节点
        levels = {}
        node_dict = {node["id"]: node for node in tree_state["nodes"]}
        
        # 构建父子关系
        child_to_parent = {}
        for link in tree_state["links"]:
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
    
    def render_frame(self, frame_idx):
        """渲染一帧"""
        self.ax.clear()
        
        # 设置边界
        self.ax.set_xlim(0, 1)
        self.ax.set_ylim(0, 1)
        self.ax.axis('off')
        
        # 显示操作信息
        operation = self.tree.operations[frame_idx]
        self.ax.set_title(f"步骤 {frame_idx}: {operation}", fontsize=14)
        
        tree_state = self.tree.history[frame_idx]
        positions, node_dict = self.calculate_layout(tree_state)
        
        # 绘制节点
        for node in tree_state["nodes"]:
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
        for link in tree_state["links"]:
            source_id = link["source"]
            target_id = link["target"]
            
            if source_id in positions and target_id in positions:
                x1, y1 = positions[source_id]
                x2, y2 = positions[target_id]
                
                if link["type"] == "parent-child":
                    # 父子连接是实线
                    self.ax.plot([x1, x2], [y1, y2], 'b-', alpha=0.6)
                elif link["type"] == "leaf-link":
                    # 叶子节点链接是虚线
                    self.ax.plot([x1, x2], [y1, y2], 'r--', alpha=0.6)
    
    def animate(self):
        """生成B+树操作的动画"""
        self.fig, self.ax = plt.subplots(figsize=(12, 8))
        frames = len(self.tree.history)
        
        anim = FuncAnimation(
            self.fig, 
            self.render_frame,
            frames=frames,
            interval=2000,  # 每帧显示2秒
            repeat=True
        )
        
        plt.tight_layout()
        return anim
    
    def save_animation(self, filename='bptree_animation.mp4'):
        """保存动画到文件"""
        anim = self.animate()
        anim.save(filename, writer='ffmpeg', fps=1)
        plt.close()
    
    def show(self):
        """显示动画"""
        anim = self.animate()
        plt.show()
    
    def save_frames(self, output_dir='output'):
        """将每一帧保存为图片"""
        os.makedirs(output_dir, exist_ok=True)
        
        self.fig, self.ax = plt.subplots(figsize=(12, 8))
        
        for i in range(len(self.tree.history)):
            self.render_frame(i)
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, f'frame_{i:03d}.png'), dpi=150)
        
        plt.close()

def process_operations(operations, order=3):
    """处理B+树操作并生成可视化"""
    tree = BPTree(order=order)
    
    for op in operations:
        parts = op.strip().split()
        command = parts[0]
        
        if command == "insert":
            index, value = int(parts[1]), int(parts[2])
            tree.insert(index, value)
        
        elif command == "delete":
            index, value = int(parts[1]), int(parts[2])
            tree.delete(index, value)
        
        elif command == "find":
            index = int(parts[1])
            values = tree.find(index)
            # 查找操作不会改变树，但会记录状态
    
    return tree

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='B+树操作可视化工具')
    parser.add_argument('-f', '--file', type=str, help='输入操作文件')
    parser.add_argument('-o', '--output', type=str, default='output', help='输出目录')
    parser.add_argument('--order', type=int, default=3, help='B+树的阶，默认为3')
    parser.add_argument('--save-frames', action='store_true', help='保存每一帧为图片')
    parser.add_argument('--save-animation', action='store_true', help='保存动画为视频文件')
    parser.add_argument('--show', action='store_true', help='显示动画')
    args = parser.parse_args()
    
    # 如果没有输入文件，从标准输入读取
    if args.file:
        with open(args.file, 'r') as f:
            n = int(f.readline().strip())
            operations = [f.readline().strip() for _ in range(n)]
    else:
        n = int(input().strip())
        operations = [input().strip() for _ in range(n)]
    
    # 处理操作并生成B+树
    tree = process_operations(operations, order=args.order)
    
    # 创建可视化器
    visualizer = BPTreeVisualizer(tree)
    
    # 输出选项
    if args.save_frames:
        visualizer.save_frames(args.output)
        print(f"已保存帧图片到 {args.output} 目录")
    
    if args.save_animation:
        output_file = os.path.join(args.output, 'bptree_animation.mp4')
        os.makedirs(args.output, exist_ok=True)
        visualizer.save_animation(output_file)
        print(f"已保存动画到 {output_file}")
    
    if args.show:
        visualizer.show()
    
    # 如果没有指定任何输出选项，默认显示动画
    if not (args.save_frames or args.save_animation or args.show):
        visualizer.show()

if __name__ == "__main__":
    main()