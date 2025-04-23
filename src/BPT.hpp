#include "MemoryRiver.hpp"
#include "utility"
#include "utility.hpp"
#include "vector.hpp"
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
  Key_Value<Key, Value> keys[DEFAULT_ORDER];
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

 private:
  std::string filename_;
  MemoryRiver<Index<Key, Value>, 2> index_file_;
  MemoryRiver<Block<Key, Value>, 2> block_file_;

  // search for target leafnode and record the search path
  int findLeafNode(const Key_Value<Key, Value> &key, sjtu::vector<sjtu::pair<int, int>> &path);

  // insert key-value pair and return true if need split
  bool insertIntoLeaf(int leaf_addr, const Key &key, const Value &value,
                      Key_Value<Key, Value> &split_key, int &new_leaf_addr);

  // handle split logic
  bool splitLeaf(Block<Key, Value> &leaf, int leaf_addr, Key_Value<Key, Value> &split_key,
                 int &new_leaf_addr);

  // pass the split information to parent node
  bool insertIntoParent(const sjtu::vector<sjtu::pair<int, int>> &path,
                        int level, const Key_Value<Key, Value> &key, int right_child);

  // split index node
  bool splitInternal(Index<Key, Value> &node, int node_addr, Key_Value<Key, Value> &split_key,
                  int &new_node_addr);

  void balanceAfterRemove(Block<Key, Value> &node, int node_addr, sjtu::vector<sjtu::pair<int,int>> &path);

  void removeFromParent(Index<Key, Value> &parent, int parent_addr, int key_idx, sjtu::vector<sjtu::pair<int,int>> &path);

  void balanceInternalNode(Index<Key, Value> &node, int node_addr, sjtu::vector<sjtu::pair<int, int>> &path);
  
};