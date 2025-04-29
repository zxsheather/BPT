#include "BPT.hpp"



template <class Key, class Value>
void BPT<Key, Value>::insert(const Key &key, const Value &value) {
  if (root_ == -1) {
    Block<Key, Value> new_block;
    new_block.data[0] = Key_Value<Key, Value>{key, value};
    new_block.size++;
    new_block.next = -1;
    //int head_ = block_file_.write(new_block);
    int head_ = cache_manager_.write_block(new_block);
    root_ = head_;
    block_file_.write_info(head_, 1);
    index_file_.write_info(root_, 1);
    height_ = 0;
    return;
  }

  sjtu::vector<pathFrame<Key, Value>> path;
  int leaf_addr = findLeafNode({key, value}, path);

  Key_Value<Key, Value> split_key;
  int new_leaf_addr;
  bool leaf_split =
      insertIntoLeaf(leaf_addr, key, value, split_key, new_leaf_addr);

  if (leaf_split) {
    insertIntoParent(path, path.size() - 1, split_key, new_leaf_addr);
  }
}

template <class Key, class Value>
void BPT<Key, Value>::remove(const Key &key, const Value &value) {
  sjtu::vector<pathFrame<Key, Value>> path;
  Key_Value<Key, Value> kv = Key_Value<Key, Value>{key, value};
  int leaf_addr = findLeafNode(kv, path);
  if (leaf_addr == -1) {
    return;
  }
  Block<Key, Value> leaf;
  //block_file_.read(leaf, leaf_addr);
  cache_manager_.read_block(leaf, leaf_addr);
  int pos = -1;
  pos = leaf.size == 0 ? 0 : binarySearch(leaf.data, kv, 0, leaf.size - 1);
  if (pos >= leaf.size || leaf.data[pos] != kv) {
    return;
  }
  for (int i = pos; i < leaf.size - 1; ++i) {
    leaf.data[i] = leaf.data[i + 1];
  }
  leaf.size--;
  if (leaf.size >= (DEFAULT_LEAF_SIZE + 1) / 3) {
    //block_file_.update(leaf, leaf_addr);
    cache_manager_.update_block(leaf, leaf_addr);
    return;
  }
  balanceAfterRemove(leaf, leaf_addr, path);
}

template <class Key, class Value>
sjtu::vector<Value> BPT<Key, Value>::find(const Key &key) {
  sjtu::vector<Value> result;
  size_t level = 1;
  int ptr = root_;
  if (ptr == -1) {
    return result;
  }
  if(height_ >= 1){
    while (level < height_) {
      Index<Key, Value> index;
      // index_file_.read(index, ptr);
      cache_manager_.read_index(index, ptr);
      int idx = binarySearch(index.keys, key, 0, index.size - 1);
      ptr = index.children[idx];
      level++;
    }
    Index<Key, Value> index;
    //index_file_.read(index, ptr);
    cache_manager_.read_index(index, ptr);
    int idx = binarySearch(index.keys, key, 0, index.size - 1);
    ptr = index.children[idx];
  }
  
  Block<Key, Value> block;
  //block_file_.read(block, ptr);
  cache_manager_.read_block(block, ptr);
  int idx = binarySearch(block.data, key, 0, block.size - 1);
  if (idx >= block.size) {
    ptr = block.next;
    if (ptr == -1) {
      return result;
    }
    //block_file_.read(block, ptr);
    cache_manager_.read_block(block, ptr);
    idx = 0;
  } else if (block.data[idx].key > key) {
    return result;
  }
  while (true) {
    if (idx == block.size) {
      ptr = block.next;
      if (ptr == -1) {
        return result;
      }
      //block_file_.read(block, ptr);
      cache_manager_.read_block(block, ptr);
      idx = 0;
    }
    if (block.data[idx].key > key) {
      return result;
    }
    result.push_back(block.data[idx].value);
    ++idx;
  }
}

template <class Key, class Value>
int BPT<Key, Value>::findLeafNode(const Key_Value<Key, Value> &key,
                                  sjtu::vector<pathFrame<Key, Value>> &path) {
  int ptr = root_;
  path.clear();
  if (ptr == -1) {
    return -1;
  }
  for (int level = 1; level <= height_; level++) {
    Index<Key, Value> node;
    // index_file_.read(node, ptr);
    cache_manager_.read_index(node, ptr);
    int idx =
        (node.size == 0) ? 0 : binarySearchForBigOrEqual(node.keys, key, 0, node.size - 1);
    path.push_back({node, ptr, idx});
    ptr = node.children[idx];
  }
  return ptr;
}



template <class Key, class Value>
bool BPT<Key, Value>::insertIntoLeaf(int leaf_addr, const Key &key,
                                     const Value &value,
                                     Key_Value<Key, Value> &split_key,
                                     int &new_leaf_addr) {
  Block<Key, Value> leaf;
  //block_file_.read(leaf, leaf_addr);
  cache_manager_.read_block(leaf, leaf_addr);
  
  int pos = (leaf.size == 0)
                ? 0
                : binarySearch(leaf.data, {key, value}, 0, leaf.size - 1);
  for (int i = leaf.size; i > pos; --i) {
    leaf.data[i] = leaf.data[i - 1];
  }
  leaf.data[pos] = Key_Value<Key, Value>{key, value};
  leaf.size++;
  //block_file_.update(leaf, leaf_addr);
  cache_manager_.update_block(leaf, leaf_addr);
  
  if (leaf.size == DEFAULT_LEAF_SIZE + 1) {
    return splitLeaf(leaf, leaf_addr, split_key, new_leaf_addr);
  }
  return false;
}

template <class Key, class Value>
bool BPT<Key, Value>::splitLeaf(Block<Key, Value> &leaf, int leaf_addr,
                                Key_Value<Key, Value> &split_key,
                                int &new_leaf_addr) {
  int mid = (DEFAULT_LEAF_SIZE + 1) / 2;
  Block<Key, Value> new_leaf;
  new_leaf.size = DEFAULT_LEAF_SIZE + 1 - mid;
  for (int i = 0; i < new_leaf.size; ++i) {
    new_leaf.data[i] = leaf.data[i + mid];
  }
  leaf.size = mid;
  new_leaf.next = leaf.next;
  split_key = new_leaf.data[0];
  //new_leaf_addr = block_file_.write(new_leaf);
  new_leaf_addr = cache_manager_.write_block(new_leaf);
  leaf.next = new_leaf_addr;
  //block_file_.update(leaf, leaf_addr);
  cache_manager_.update_block(leaf, leaf_addr);
  return true;
}

template <class Key, class Value>
bool BPT<Key, Value>::insertIntoParent(
    const sjtu::vector<pathFrame<Key, Value>> &path, int level,
    const Key_Value<Key, Value> &key, int right_child) {
  if (level < 0) {
    Index<Key, Value> new_root;
    new_root.size = 1;
    new_root.keys[0] = key;
    new_root.children[0] = path.empty() ? root_ : path[0].index_addr;
    new_root.children[1] = right_child;
    //root_ = index_file_.write(new_root);
    root_ = cache_manager_.write_index(new_root);
    index_file_.write_info(root_, 1);
    height_++;
    index_file_.write_info(height_ , 2);
    return true;
  }
  auto [parent, parent_addr, child_idx] = path[level];

  for (int i = parent.size; i > child_idx; --i) {
    parent.keys[i] = parent.keys[i - 1];
    parent.children[i + 1] = parent.children[i];
  }
  parent.keys[child_idx] = key;
  parent.children[child_idx + 1] = right_child;
  parent.size++;
  if (parent.size < DEFAULT_ORDER) {
    //index_file_.update(parent, parent_addr);
    cache_manager_.update_index(parent, parent_addr);
    return false;
  }

  Key_Value<Key, Value> new_split_key;
  int new_index_addr;
  bool result =
      splitInternal(parent, parent_addr, new_split_key, new_index_addr);
  if (result) {
    return insertIntoParent(path, level - 1, new_split_key, new_index_addr);
  }
  return false;
}

template <class Key, class Value>
bool BPT<Key, Value>::splitInternal(Index<Key, Value> &node, int node_addr,
                                    Key_Value<Key, Value> &split_key,
                                    int &new_node_addr) {
  Index<Key, Value> new_node;
  int split_pos = DEFAULT_ORDER / 2;
  new_node.size = DEFAULT_ORDER - split_pos - 1;
  for (int i = 0; i < new_node.size; ++i) {
    new_node.keys[i] = node.keys[i + split_pos + 1];
    new_node.children[i] = node.children[i + split_pos + 1];
  }
  new_node.children[new_node.size] = node.children[DEFAULT_ORDER];
  split_key = node.keys[split_pos];
  node.size = split_pos;
  //index_file_.update(node, node_addr);
  //new_node_addr = index_file_.write(new_node);
  cache_manager_.update_index(node, node_addr);
  new_node_addr = cache_manager_.write_index(new_node);
  return true;
}

template <class Key, class Value>
void BPT<Key, Value>::balanceAfterRemove(
    Block<Key, Value> &node, int node_addr,
    sjtu::vector<pathFrame<Key, Value>> &path) {
  if (path.empty()) {
    if (node.size == 0) {
      root_ = -1;
      height_ = 0;
      index_file_.write_info(-1, 1);
      index_file_.write_info(0, 2);
      return;
    } else {
      //block_file_.update(node, node_addr);
      cache_manager_.update_block(node, node_addr);
      return;
    }
  }
  auto [parent, parent_addr, child_idx] = path.back();
  path.pop_back();
  Block<Key, Value> left_sibling;
  int left_sibling_addr;
  if (child_idx >= 1) {
    left_sibling_addr = parent.children[child_idx - 1];
    //block_file_.read(left_sibling, left_sibling_addr);
    cache_manager_.read_block(left_sibling, left_sibling_addr);
    if (left_sibling.size > (DEFAULT_LEAF_SIZE + 1) / 2) {
      for (int i = node.size; i >= 1; --i) {
        node.data[i] = node.data[i - 1];
      }
      node.data[0] = left_sibling.data[left_sibling.size - 1];
      node.size++;
      left_sibling.size--;
      parent.keys[child_idx - 1] = node.data[0];
      //block_file_.update(node, node_addr);
      //block_file_.update(left_sibling, left_sibling_addr);
      //index_file_.update(parent, parent_addr);
      cache_manager_.update_block(node, node_addr);
      cache_manager_.update_block(left_sibling, left_sibling_addr);
      cache_manager_.update_index(parent, parent_addr);
      return;
    }
  }
  Block<Key, Value> right_sibling;
  int right_sibling_addr;
  if (child_idx <= parent.size - 1) {
    right_sibling_addr = parent.children[child_idx + 1];
    //block_file_.read(right_sibling, right_sibling_addr);
    cache_manager_.read_block(right_sibling, right_sibling_addr);
    if (right_sibling.size > (DEFAULT_LEAF_SIZE + 1) / 2) {
      node.data[node.size] = right_sibling.data[0];
      for (int i = 0; i <= right_sibling.size - 2; ++i) {
        right_sibling.data[i] = right_sibling.data[i + 1];
      }
      node.size++;
      right_sibling.size--;
      parent.keys[child_idx] = right_sibling.data[0];
      //block_file_.update(node, node_addr);
      //block_file_.update(right_sibling, right_sibling_addr);
      //index_file_.update(parent, parent_addr);
      cache_manager_.update_block(node, node_addr);
      cache_manager_.update_block(right_sibling, right_sibling_addr);
      cache_manager_.update_index(parent, parent_addr);
      return;
    }
  }

  if (child_idx >= 1) {
    for (int i = 0; i < node.size; ++i) {
      left_sibling.data[left_sibling.size + i] = node.data[i];
    }
    left_sibling.size += node.size;
    left_sibling.next = node.next;
    //block_file_.update(left_sibling, left_sibling_addr);
    cache_manager_.update_block(left_sibling, left_sibling_addr);
    removeFromParent(parent, parent_addr, child_idx - 1, path);
  } else if (child_idx <= parent.size - 1) {
    for (int i = 0; i < right_sibling.size; ++i) {
      node.data[node.size + i] = right_sibling.data[i];
    }
    node.size += right_sibling.size;
    node.next = right_sibling.next;
    //block_file_.update(node, node_addr);
    cache_manager_.update_block(node, node_addr);
    removeFromParent(parent, parent_addr, child_idx, path);
  }
}

template <class Key, class Value>
void BPT<Key, Value>::removeFromParent(
    Index<Key, Value> &parent, int parent_addr, int key_idx,
    sjtu::vector<pathFrame<Key, Value>> &path) {
  for (int i = key_idx; i < parent.size - 1; ++i) {
    parent.keys[i] = parent.keys[i + 1];
  }
  for (int i = key_idx + 1; i < parent.size; ++i) {
    parent.children[i] = parent.children[i + 1];
  }
  parent.size--;
  if (path.empty() && parent.size == 0) {
    root_ = parent.children[0];
    height_ --;
    index_file_.write_info(root_, 1);
    index_file_.write_info(height_ , 2);
    return;
  }
  if (path.empty() || parent.size >= DEFAULT_ORDER / 3) {
    //index_file_.update(parent, parent_addr);
    cache_manager_.update_index(parent, parent_addr);
    return;
  }
  balanceInternalNode(parent, parent_addr, path);
}

template <class Key, class Value>
void BPT<Key, Value>::balanceInternalNode(
    Index<Key, Value> &node, int node_addr,
    sjtu::vector<pathFrame<Key, Value>> &path) {
  auto [parent, parent_addr, node_idx] = path.back();
  path.pop_back();
  Index<Key, Value> left_sibling;
  int left_sibling_addr;
  if (node_idx >= 1) {
    left_sibling_addr = parent.children[node_idx - 1];
    //index_file_.read(left_sibling, left_sibling_addr);
    cache_manager_.read_index(left_sibling, left_sibling_addr);

    if (left_sibling.size > DEFAULT_ORDER / 2) {
      for (int i = node.size; i > 0; --i) {
        node.keys[i] = node.keys[i - 1];
      }
      for (int i = node.size + 1; i > 0; --i) {
        node.children[i] = node.children[i - 1];
      }
      node.keys[0] = parent.keys[node_idx - 1];
      node.children[0] = left_sibling.children[left_sibling.size];
      parent.keys[node_idx - 1] = left_sibling.keys[left_sibling.size - 1];
      node.size++;
      left_sibling.size--;
      //index_file_.update(node, node_addr);
      //index_file_.update(left_sibling, left_sibling_addr);
      //index_file_.update(parent, parent_addr);
      cache_manager_.update_index(node, node_addr);
      cache_manager_.update_index(left_sibling, left_sibling_addr);
      cache_manager_.update_index(parent, parent_addr);
      return;
    }
  }

  Index<Key, Value> right_sibling;
  int right_sibling_addr;
  if (node_idx <= parent.size - 1) {
    right_sibling_addr = parent.children[node_idx + 1];
    //index_file_.read(right_sibling, right_sibling_addr);
    cache_manager_.read_index(right_sibling, right_sibling_addr);

    if (right_sibling.size > DEFAULT_ORDER / 2) {
      node.keys[node.size] = parent.keys[node_idx];
      node.children[node.size + 1] = right_sibling.children[0];
      parent.keys[node_idx] = right_sibling.keys[0];
      node.size++;
      for (int i = 0; i < right_sibling.size - 1; ++i) {
        right_sibling.keys[i] = right_sibling.keys[i + 1];
      }
      for (int i = 0; i < right_sibling.size; ++i) {
        right_sibling.children[i] = right_sibling.children[i + 1];
      }
      right_sibling.size--;
      //index_file_.update(node, node_addr);
      //index_file_.update(right_sibling, right_sibling_addr);
      //index_file_.update(parent, parent_addr);
      cache_manager_.update_index(node, node_addr);
      cache_manager_.update_index(right_sibling, right_sibling_addr);
      cache_manager_.update_index(parent, parent_addr);
      return;
    }
  }

  if (node_idx >= 1) {
    left_sibling.keys[left_sibling.size] = parent.keys[node_idx - 1];
    for (int i = 0; i < node.size; ++i) {
      left_sibling.keys[left_sibling.size + 1 + i] = node.keys[i];
    }
    for (int i = 0; i <= node.size; ++i) {
      left_sibling.children[left_sibling.size + 1 + i] = node.children[i];
    }
    left_sibling.size += node.size + 1;
    //index_file_.update(left_sibling, left_sibling_addr);
    cache_manager_.update_index(left_sibling, left_sibling_addr);
    removeFromParent(parent, parent_addr, node_idx - 1, path);
  } else if (node_idx <= parent.size - 1) {
    node.keys[node.size] = parent.keys[node_idx];
    for (int i = 0; i < right_sibling.size; ++i) {
      node.keys[node.size + 1 + i] = right_sibling.keys[i];
    }
    for (int i = 0; i <= right_sibling.size; ++i) {
      node.children[node.size + 1 + i] = right_sibling.children[i];
    }
    node.size += right_sibling.size + 1;
    //index_file_.update(node, node_addr);
    cache_manager_.update_index(node, node_addr);
    removeFromParent(parent, parent_addr, node_idx , path);
  }
}

template class BPT<int, int>;