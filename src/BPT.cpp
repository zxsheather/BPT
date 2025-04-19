#include "BPT.hpp"

#include <fstream>

#include "exceptions.hpp"
#include "utility.hpp"

template <class Key, class Value>
void BPT<Key, Value>::insert(const Key &key, const Value &value) {
  int root_;
  index_file_.get_info(root_, 1);
  if (root_ == -1) {
    Index<Key, Value> new_root;
    Block<Key, Value> new_block;
    new_block.data[0] = Key_Value<Key, Value>{key, value};
    new_root.keys[0] = key;
    new_root.size++;
    new_block.size++;
    new_block.next = -1;
    long head_ = block_file_.write(new_block);
    new_root.children[0] = head_;
    root_ = index_file_.write(new_root);
    block_file_.write_info(head_, 1);
    index_file_.write_info(root_, 1);
    index_file_.write_info(1, 2);
    block_file_.write_info(1, 2);
    return;
  }
}

template <class Key, class Value>
sjtu::vector<Value> BPT<Key, Value>::find(const Key &key) {
  sjtu::vector<Value> result;
  size_t level = 1;
  int ptr;
  index_file_.get_info(ptr, 1);
  int height;
  index_file_.get_info(ptr, 2);
  if (ptr == -1) {
    return result;
  }
  while (level != height) {
    Index<Key, Value> index;
    index_file_.read(index, ptr);
    int idx = binarySearch(index.keys, key, 0, index.size-1);
    ptr = index.children[idx];
    level++;
  }
  Index<Key, Value> index;
  index_file_.read(index, ptr);
  int idx = binarySearch(index.keys, key, 0, index.size-1);
  ptr = index.children[idx];

  Block<Key, Value> block;
  block_file_.read(block, ptr);
  idx = binarySearch(block.data, key, 0, block.size-1);
  if(idx>=block.size||block.data[idx].key>key){
    return result;
  }
  while(true){
    if(idx==block.size){
      ptr = block.next;
      if(ptr==-1){
        return result;
      }
      block_file_.read(block, ptr);
      idx = 0;
    }
    if(block.data[idx].key > key){
      return result;
    }
    result.push_back(block.data[idx].value);
    ++idx;
  }
  
  
}

template class BPT<int, int>;