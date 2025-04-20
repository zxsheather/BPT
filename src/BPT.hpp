#include "MemoryRiver.hpp"
#include "utility"
#include "utility.hpp"
#include "vector.hpp"

constexpr size_t DEFAULT_ORDER = 4;
constexpr size_t DEFAULT_LEAF_SIZE = 4;

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
};

template <class Key, class Value>
struct Index {
  long children[DEFAULT_ORDER];
  Key keys[DEFAULT_ORDER - 1];
  size_t size;

  Index() : size(0) {
    for (size_t i = 0; i < DEFAULT_ORDER; ++i) {
      children[i] = -1;
    }
  }
};

template <class Key, class Value>
struct Block {
  long next;
  Key_Value<Key, Value> data[DEFAULT_LEAF_SIZE];
  size_t size;

  Block() : next(-1), size(0) {
    for (size_t i = 0; i < DEFAULT_LEAF_SIZE; ++i) {
      data[i] = Key_Value<Key, Value>{};
    }
  }
};

template <class Key, class Value>
int binarySearch(Key_Value<Key, Value> *array, const Key &key, int left,
                 int right) {
  if (array[left].key >= key) {
    return left;
  }
  if (array[right].key < key) {
    return right + 1;
  }
  while (left < right) {
    int mid = (left + right + 1) / 2;
    if (array[mid].key < key) {
      left = mid;
    } else {
      right = mid - 1;
    }
  }
  return left + 1;
}

template <class Key>
int binarySearch(Key *array, const Key &key, int left, int right) {
  if (array[left] >= key) {
    return left;
  }
  if (array[right] < key) {
    return right + 1;
  }
  while (left < right) {
    int mid = (left + right + 1) / 2;
    if (array[mid] < key) {
      left = mid;
    } else {
      right = mid - 1;
    }
  }
  return left + 1;
}

template <class Key, class Value>
class BPT {
 public:
  BPT(const std::string &filename = "database")
      : filename_(filename),
        index_file_(filename + ".index"),
        block_file_(filename + ".block") {
    if (!index_file_.exist()) {
      index_file_.initialise();
      block_file_.initialise();
      index_file_.write_info(-1, 1);
      block_file_.write_info(-1, 1);
      index_file_.write_info(0, 2);
      block_file_.write_info(0, 2);
    }
  }
  ~BPT() = default;
  void insert(const Key &key, const Value &value);
  void remove(const Key &key);
  sjtu::vector<Value> find(const Key &key);

 private:
  std::string filename_;
  MemoryRiver<Index<Key, Value>, 2> index_file_;
  MemoryRiver<Block<Key, Value>, 2> block_file_;

  // search for target leafnode and record the search path
  long findLeafNode(const Key &key, sjtu::vector<sjtu::pair<long, int>> &path);

  // insert key-value pair and return true if need split
  bool insertIntoLeaf(long leaf_addr, const Key &key, const Value &value,
                      Key &split_key, long &new_leaf_addr);

  // handle split logic
  bool splitLeaf(Block<Key, Value> &leaf, long leaf_addr, int pos,
                 const Key &key, const Value &value, long &new_leaf_addr);

  // pass the split information to parent node
  bool insertIntoParent(const sjtu::vector<sjtu::pair<long, int>> &path,
                        int level, const Key &key, long right_child);

  // split index node
  bool splitIndex(Index<Key, Value> &node, long node_addr, int pos,
                  const Key &key, long right_child, Key &split_key,
                  long &new_node_addr);
};