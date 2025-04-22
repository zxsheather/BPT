#include "MemoryRiver.hpp"
#include "utility"
#include "utility.hpp"
#include "vector.hpp"
#include <iostream>
#include <string>

constexpr size_t DEFAULT_ORDER = 4;
constexpr size_t DEFAULT_LEAF_SIZE = 2;

template <class Key, class Value>
struct Key_Value {
  Key key;
  Value value;

  bool operator<(const Key_Value &other) const {
    if (key == other.key) {
      return value < other.value;
    }
    return key < other.key;
  }

  bool operator>(const Key_Value &other) const {
    if (key == other.key) {
      return value > other.value;
    }
    return key > other.key;
  }

  bool operator==(const Key_Value &other) const {
    return key == other.key && value == other.value;
  }

  bool operator!=(const Key_Value &other) const {
    return !(*this == other);
  }

  bool operator<=(const Key_Value &other) const {
    return *this < other || *this == other;
  }
};

//Increment the size of keys to facilitate split
template <class Key, class Value>
struct Index {
  int children[DEFAULT_ORDER+1];
  Key keys[DEFAULT_ORDER];
  size_t size;

  Index() : size(0) {
    for (size_t i = 0; i < DEFAULT_ORDER; ++i) {
      children[i] = -1;
    }
  }
};

template <class Key, class Value>
struct Block {
  int next;
  Key_Value<Key, Value> data[DEFAULT_LEAF_SIZE + 1];
  size_t size;

  Block() : next(-1), size(0) {
    for (size_t i = 0; i < DEFAULT_LEAF_SIZE; ++i) {
      data[i] = Key_Value<Key, Value>{};
    }
  }
};

template <class Key, class Value>
int binarySearch(Key_Value<Key, Value> *array, const Key &key, int left, int right) {
  if (left > right || left < 0) return 0;
  
  // 处理边界情况
  if (key <= array[left].key) return left;
  if (key > array[right].key) return right + 1;
  
  // 二分查找
  int l = left, r = right;
  while (l < r) {
    int mid = l + (r - l) / 2;
    if (array[mid].key < key) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  return l;
}

template <class Key>
int binarySearch(Key *array, const Key &key, int left, int right) {
  if (left > right || left < 0) return 0;
  
  if (key <= array[left]) return left;
  if (key > array[right]) return right + 1;
  
  int l = left, r = right;
  while (l < r) {
    int mid = l + (r - l) / 2;
    if (array[mid] < key) {
      l = mid + 1;
    } else {
      r = mid;
    }
  }
  return l;
}

template <typename T>
void Qsort(sjtu::vector<T>& v, int l, int r){
  if (l >= r) return;
  T pivot = v[(l + r) / 2];
  int i = l, j = r;
  while (i <= j) {
    while (v[i] < pivot) i++;
    while (v[j] > pivot) j--;
    if (i <= j) {
      std::swap(v[i], v[j]);
      i++;
      j--;
    }
  }
  Qsort(v, l, j);
  Qsort(v, i, r);
  return;
}

template <class Key, class Value>
class BPT {
 public:
  BPT(const std::string &filename = "database")
  : filename_(filename),
        index_file_(filename + ".index"),
        block_file_(filename + ".block") {
    //if (!index_file_.exist()) {
      index_file_.initialise();
      block_file_.initialise();
      index_file_.write_info(-1, 1);
      block_file_.write_info(-1, 1);
      index_file_.write_info(0, 2);
      block_file_.write_info(0, 2);
    //}
  }
  ~BPT() = default;
  void insert(const Key &key, const Value &value);
  void remove(const Key &key, const Value &value);
  sjtu::vector<Value> find(const Key &key);
  
  // 打印树结构的函数
  void printTree() {
    int root_addr;
    int height;
    index_file_.get_info(root_addr, 1);
    index_file_.get_info(height, 2);
    
    if (root_addr == -1) {
      std::cout << "Empty Tree" << std::endl;
      return;
    }
    
    std::cout << "================ B+ Tree Structure ================\n";
    std::cout << "Tree Height: " << height << std::endl;
    
    // 递归打印树结构
    printNode(root_addr, 0, height);
    
    // 打印叶子节点链表
    printLeafChain();
    
    std::cout << "==================================================\n";
  }

  void debugSearchPath(const Key&);

 private:
  std::string filename_;
  MemoryRiver<Index<Key, Value>, 2> index_file_;
  MemoryRiver<Block<Key, Value>, 2> block_file_;

  // search for target leafnode and record the search path
  int findLeafNode(const Key &key, sjtu::vector<sjtu::pair<int, int>> &path);

  // insert key-value pair and return true if need split
  bool insertIntoLeaf(int leaf_addr, const Key &key, const Value &value,
                      Key &split_key, int &new_leaf_addr);

  // handle split logic
  bool splitLeaf(Block<Key, Value> &leaf, int leaf_addr, Key &split_key,
                 int &new_leaf_addr);

  // pass the split information to parent node
  bool insertIntoParent(const sjtu::vector<sjtu::pair<int, int>> &path,
                        int level, const Key &key, int right_child);

  // split index node
  bool splitInternal(Index<Key, Value> &node, int node_addr, Key &split_key,
                  int &new_node_addr);
  
  // 打印节点的辅助函数
  void printNode(int node_addr, int level, int height) {
    std::string indent(level * 4, ' ');
    
    if (level == height - 1) {
      // 叶子节点
      Block<Key, Value> leaf;
      block_file_.read(leaf, node_addr);
      
      std::cout << indent << "Leaf Node [addr=" << node_addr 
                << ", size=" << leaf.size << ", next=" << leaf.next << "]" << std::endl;
      
      std::cout << indent << "  ";
      for (size_t i = 0; i < leaf.size; i++) {
        std::cout << "(" << leaf.data[i].key << ":" << leaf.data[i].value << ") ";
      }
      std::cout << std::endl;
    } else {
      // 内部节点
      Index<Key, Value> node;
      index_file_.read(node, node_addr);
      
      std::cout << indent << "Internal Node [addr=" << node_addr 
                << ", size=" << node.size << "]" << std::endl;
      
      // 打印键
      std::cout << indent << "  Keys: ";
      for (size_t i = 0; i < node.size; i++) {
        std::cout << node.keys[i] << " ";
      }
      std::cout << std::endl;
      
      // 打印子节点指针
      std::cout << indent << "  Ptrs: ";
      for (size_t i = 0; i <= node.size; i++) {
        std::cout << node.children[i] << " ";
      }
      std::cout << std::endl;
      
      // 递归打印子节点
      for (size_t i = 0; i <= node.size; i++) {
        printNode(node.children[i], level + 1, height);
      }
    }
  }
  
  // 打印叶子节点链表
  void printLeafChain() {
    int head_addr;
    block_file_.get_info(head_addr, 1);
    
    if (head_addr == -1) {
      std::cout << "Empty leaf chain" << std::endl;
      return;
    }
    
    std::cout << "Leaf Chain: ";
    int current = head_addr;
    int count = 0;
    const int max_print = 10; // 限制打印数量，避免链表过长
    
    while (current != -1 && count < max_print) {
      // std::cout << current;
      
      Block<Key, Value> leaf;
      block_file_.read(leaf, current);
      std::cout << "[" << current << ": ";
      for (size_t i = 0; i < leaf.size; i++) {
        std::cout << "(" << leaf.data[i].key << ":" << leaf.data[i].value << ") ";
      }
      std::cout << "]";
      current = leaf.next;
      
      if (current != -1) std::cout << " -> ";
      count++;
    }
    
    if (current != -1) std::cout << " -> ...";
    std::cout << std::endl;
  }
};