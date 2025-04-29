#pragma once
#include <string>

#include "MemoryRiver.hpp"
#include "cache.hpp"
#include "vector.hpp"
#include "IndexBlock.hpp"

template <class Key, class Value>
struct pathFrame {
  Index<Key, Value> index;
  int index_addr;
  int pos;
};

template <class Key, class Value>
int binarySearch(Key_Value<Key, Value> *array, const Key &key, int left,
                 int right) {
  if (key <= array[left].key) return left;
  if (key > array[right].key) return right + 1;

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

template <class Key>
int binarySearchForBigOrEqual(Key *array, const Key &key, int left, int right) {
  if (key < array[left]) return left;
  if (key >= array[right]) return right + 1;

  int l = left, r = right;
  while (l < r) {
    int mid = l + (r - l) / 2;
    if (array[mid] < key) {
      l = mid + 1;
    } else if (array[mid] > key) {
      r = mid;
    } else {
      return mid + 1;
    }
  }
  return l;
}

template <class Key, class Value>
class BPT {
 public:
  BPT(const std::string &filename = "database")
      : filename_(filename),
        index_file_(filename + ".index"),
        block_file_(filename + ".block"),
        cache_manager_(index_file_, block_file_) {
    if (!index_file_.exist()) {
      index_file_.initialise();
      block_file_.initialise();
      // index_file_.write_info(-1, 1);
      // block_file_.write_info(-1, 1);
      // index_file_.write_info(0, 2);
      // block_file_.write_info(0, 2);
      root_ = -1;
      height_ = 0;
    } else {
      index_file_.get_info(root_, 1);
      index_file_.get_info(height_, 2);
    }
  }
  ~BPT(){
    cache_manager_.flush_cache();
    index_file_.write_info(root_, 1);
    index_file_.write_info(height_,2);
  }
  void insert(const Key &key, const Value &value);
  void remove(const Key &key, const Value &value);
  sjtu::vector<Value> find(const Key &key);

 private:
  std::string filename_;
  MemoryRiver<Index<Key, Value>, 2> index_file_;
  MemoryRiver<Block<Key, Value>, 2> block_file_;
  int root_;
  int height_;
  sjtu::BPTCacheManager<Key, Value> cache_manager_;

  // search for target leafnode and record the search path
  int findLeafNode(const Key_Value<Key, Value> &key,
                   sjtu::vector<pathFrame<Key, Value>> &path);

  // insert key-value pair and return true if need split
  bool insertIntoLeaf(int leaf_addr, const Key &key, const Value &value,
                      Key_Value<Key, Value> &split_key, int &new_leaf_addr);

  // handle split logic
  bool splitLeaf(Block<Key, Value> &leaf, int leaf_addr,
                 Key_Value<Key, Value> &split_key, int &new_leaf_addr);

  // pass the split information to parent node
  bool insertIntoParent(const sjtu::vector<pathFrame<Key, Value>> &path,
                        int level, const Key_Value<Key, Value> &key,
                        int right_child);

  // split index node
  bool splitInternal(Index<Key, Value> &node, int node_addr,
                     Key_Value<Key, Value> &split_key, int &new_node_addr);

  // balance block by borrowing from siblings or merge
  void balanceAfterRemove(Block<Key, Value> &node, int node_addr,
                          sjtu::vector<pathFrame<Key, Value>> &path);

  // adjust parent index after block merging
  void removeFromParent(Index<Key, Value> &parent, int parent_addr, int key_idx,
                        sjtu::vector<pathFrame<Key, Value>> &path);

  // adjust parent index after index merging
  void balanceInternalNode(Index<Key, Value> &node, int node_addr,
                           sjtu::vector<pathFrame<Key, Value>> &path);
};