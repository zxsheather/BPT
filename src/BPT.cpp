#include "BPT.hpp"

#include <fstream>

#include "exceptions.hpp"
#include "utility.hpp"


template <class Key, class Value>
void BPT<Key, Value>::insert(const Key &key, const Value &value) {
  if (root_ == -1) {
    Index<Key, Value> new_root;
    Block<Key, Value> new_block;
    new_block.data = Key_Value<Key, Value>{key, value};
    new_root.keys[0] = key;
    new_root.size++;
    new_block.size++;
		height_++;
    head_ = block_file_.write(new_block);
    new_root.children[0] = head_;
    root_ = index_file_.write(new_root);
		block_file_.write_info(head_, 1);
		index_file_.write_info(root_, 1);
		index_file_.write_info(height_, 2);
		return;
  }
	long ptr = root_;
	
}

template class BPT<int, int>;