#include "BPT.hpp"

#include "utility.hpp"


template <class Key, class Value>
void BPT<Key, Value>::insert(const Key &key, const Value &value) {
  int ptr;
  index_file_.get_info(ptr, 1);
  if (ptr == -1) {
    Index<Key, Value> new_root;
    Block<Key, Value> new_block;
    new_block.data[0] = Key_Value<Key, Value>{key, value};
    new_block.size++;
    new_block.next = -1;
    int head_ = block_file_.write(new_block);
    new_root.children[0] = head_;
    ptr = index_file_.write(new_root);
    block_file_.write_info(head_, 1);
    index_file_.write_info(ptr, 1);
    index_file_.write_info(1, 2);
    block_file_.write_info(1, 2);
    return;
  }

  sjtu::vector<sjtu::pair<int, int>> path;
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
void BPT<Key, Value>::remove(const Key &key, const Value &value){
  sjtu::vector<sjtu::pair<int, int>> path;
}

template <class Key, class Value>
sjtu::vector<Value> BPT<Key, Value>::find(const Key &key) {
  sjtu::vector<Value> result;
  size_t level = 1;
  int ptr;
  index_file_.get_info(ptr, 1);
  int height;
  index_file_.get_info(height, 2);
  if (ptr == -1) {
    return result;
  }
  while (level != height) {
    Index<Key, Value> index;
    index_file_.read(index, ptr);
    int idx = binarySearch(index.keys, key, 0, index.size - 1);
    ptr = index.children[idx];
    level++;
  }
  Index<Key, Value> index;
  index_file_.read(index, ptr);
  int idx = binarySearch(index.keys, key, 0, index.size - 1);
  ptr = index.children[idx];

  Block<Key, Value> block;
  block_file_.read(block, ptr);
  idx = binarySearch(block.data, key, 0, block.size - 1);
  if(idx >= block.size){
    ptr = block.next;
    if (ptr == -1) {
      return result;
    }
    block_file_.read(block, ptr);
    idx = 0;
  }else if (block.data[idx].key > key) {
    return result;
  }
  while (true) {
    if (idx == block.size) {
      ptr = block.next;
      if (ptr == -1) {
        return result;
      }
      block_file_.read(block, ptr);
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
                                   sjtu::vector<sjtu::pair<int, int>> &path) {
  int ptr, height;
  index_file_.get_info(height, 2);
  index_file_.get_info(ptr, 1);
  path.clear();
  if (ptr == -1) {
    return -1;
  }
  for (int level = 1; level <= height; level++) {
    Index<Key, Value> node;
    index_file_.read(node, ptr);

    int idx =
        (node.size == 0) ? 0 : binarySearch(node.keys, key, 0, node.size - 1);
    path.push_back({ptr, idx});
    ptr = node.children[idx];
  }
  return ptr;
}

template <class Key, class Value>
bool BPT<Key, Value>::insertIntoLeaf(int leaf_addr, const Key &key,
                                     const Value &value, Key_Value<Key, Value> &split_key,
                                     int &new_leaf_addr) {
  Block<Key, Value> leaf;
  block_file_.read(leaf, leaf_addr);
  int pos =
      (leaf.size == 0) ? 0 : binarySearch(leaf.data, {key, value}, 0, leaf.size - 1);
  for (int i = leaf.size; i > pos; --i) {
    leaf.data[i] = leaf.data[i - 1];
  }
  leaf.data[pos] = Key_Value<Key, Value>{key, value};
  leaf.size++;
  block_file_.update(leaf, leaf_addr);
  if (leaf.size == DEFAULT_LEAF_SIZE + 1) {
    return splitLeaf(leaf, leaf_addr, split_key, new_leaf_addr);
  }
  return false;
}

template <class Key, class Value>
bool BPT<Key, Value>::splitLeaf(Block<Key, Value> &leaf, int leaf_addr,
                                Key_Value<Key, Value> &split_key, int &new_leaf_addr) {
  int mid = (DEFAULT_LEAF_SIZE + 1) / 2;
  Block<Key, Value> new_leaf;
  new_leaf.size = DEFAULT_LEAF_SIZE + 1 - mid;
  for (int i = 0; i < new_leaf.size; ++i) {
    new_leaf.data[i] = leaf.data[i + mid];
  }
  leaf.size = mid;
  new_leaf.next = leaf.next;
  split_key = new_leaf.data[0];
  new_leaf_addr = block_file_.write(new_leaf);
  leaf.next = new_leaf_addr;
  block_file_.update(leaf, leaf_addr);
  return true;
}

template <class Key, class Value>
bool BPT<Key, Value>::insertIntoParent(
    const sjtu::vector<sjtu::pair<int, int>> &path, int level, const Key_Value<Key, Value> &key,
    int right_child) {
  Index<Key, Value> parent;

  if (level < 0) {
    Index<Key, Value> new_root;
    new_root.size = 1;
    new_root.keys[0] = key;
    new_root.children[0] = path[0].first;
    new_root.children[1] = right_child;

    int new_root_addr = index_file_.write(new_root);
    index_file_.write_info(new_root_addr, 1);
    int height;
    index_file_.get_info(height, 2);
    index_file_.write_info(height + 1, 2);
    return true;
  }
  auto [parent_addr, child_idx] = path[level];
  index_file_.read(parent, parent_addr);

  for (int i = parent.size; i > child_idx; --i) {
    parent.keys[i] = parent.keys[i - 1];
    parent.children[i + 1] = parent.children[i];
  }
  parent.keys[child_idx] = key;
  parent.children[child_idx + 1] = right_child;
  parent.size++;
  if (parent.size < DEFAULT_ORDER) {
    index_file_.update(parent, parent_addr);
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
                                    Key_Value<Key, Value> &split_key, int &new_node_addr) {
  Index<Key, Value> new_node;
  int split_pos = DEFAULT_ORDER / 2;
  new_node.size=DEFAULT_ORDER-split_pos-1;
  for(int i=0;i<new_node.size;++i){
    new_node.keys[i]=node.keys[i+split_pos+1];
    new_node.children[i]=node.children[i+split_pos+1];
  }
  new_node.children[new_node.size]=node.children[DEFAULT_ORDER];
  split_key=node.keys[split_pos];
  node.size=split_pos;
  index_file_.update(node,node_addr);
  new_node_addr=index_file_.write(new_node);
  return true;
}


template class BPT<int, int>;