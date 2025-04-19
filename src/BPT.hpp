#include "MemoryRiver.hpp"
#include "vector.hpp"

constexpr size_t DEFAULT_ORDER = 4;
constexpr size_t DEFAULT_LEAF_SIZE = 4;

template <class Key, class Value>
struct Key_Value {
  Key key;
  Value value;

  bool operator<(const Key_Value& other) const {
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
class BPT {
 public:
  BPT(const std::string& filename = "database")
      : filename_(filename),
        index_file_(filename + ".index"),
        block_file_(filename + ".block"),
        root_(-1),
        head_(-1),
        height_(0) {
    if (!index_file_.exist()) {
      index_file_.initialise();
      block_file_.initialise();
      index_file_.write_info(-1, 1);
      block_file_.write_info(-1, 1);
      index_file_.write_info(0, 2);
    } else {
      root_ = index_file_.get_info(1);
      head_ = block_file_.get_info(1);
      height_ = index_file_.get_info(2);
    }
  }
  ~BPT() = default;
  void insert(const Key& key, const Value& value);
  void remove(const Key& key);
  sjtu::vector<Value> find(const Key& key);

 private:
  std::string filename_;
  MemoryRiver<Index<Key, Value>, 2> index_file_;
  MemoryRiver<Block<Key, Value>, 2> block_file_;
  long root_;
  long head_;
  size_t height_;
}