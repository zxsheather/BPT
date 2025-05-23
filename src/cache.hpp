
#ifndef BPT_CACHE_HPP
#define BPT_CACHE_HPP

#include <functional>

#include "HashMap.hpp"
#include "IndexBlock.hpp"
#include "MemoryRiver.hpp"
#include "list.hpp"

namespace sjtu {

template <class Key, class Value>
class LRUCache {
 private:
  struct CacheItem {
    Value value;
    bool dirty;

    CacheItem() : dirty(false) {}
    CacheItem(const Value& v, bool d = false) : value(v), dirty(d) {}
  };

  struct ItemPosition {
    typename sjtu::list<Key>::iterator position;

    ItemPosition(typename sjtu::list<Key>::iterator p) : position(p) {}
  };

  size_t capacity_;

  sjtu::list<Key> lru_list_;

  HashMap<Key, CacheItem> cache_items_;
  HashMap<Key, ItemPosition> positions_;

  using EvictionCallback = std::function<void(const Key&, const Value&)>;
  EvictionCallback eviction_callback_ = nullptr;

 public:
  explicit LRUCache(size_t capacity = 1024) : capacity_(capacity) {}

  size_t size() const { return cache_items_.size(); }

  bool empty() const { return cache_items_.empty(); }

  bool contains(const Key& key) const { return cache_items_.contains(key); }

  Value get(const Key& key) {
    if (!cache_items_.contains(key)) {
      throw std::runtime_error("Key not found in cache");
    }

    if (positions_.contains(key)) {
      lru_list_.erase(positions_.get(key).position);
    }

    lru_list_.push_front(key);
    positions_.put(key, ItemPosition(lru_list_.begin()));

    return cache_items_.get(key).value;
  }

  void put(const Key& key, const Value& value, bool dirty = true) {
    CacheItem item(value, dirty);

    if (cache_items_.contains(key)) {
      cache_items_.put(key, item);

      if (positions_.contains(key)) {
        lru_list_.erase(positions_.get(key).position);
      }

      lru_list_.push_front(key);
      positions_.put(key, ItemPosition(lru_list_.begin()));
      return;
    }

    if (cache_items_.size() >= capacity_) {
      evict();
    }

    lru_list_.push_front(key);
    cache_items_.put(key, item);
    positions_.put(key, ItemPosition(lru_list_.begin()));
  }

  void mark_dirty(const Key& key, bool is_dirty = true) {
    if (cache_items_.contains(key)) {
      CacheItem item = cache_items_.get(key);
      item.dirty = is_dirty;
      cache_items_.put(key, item);
    }
  }

  bool is_dirty(const Key& key) const {
    if (cache_items_.contains(key)) {
      return cache_items_.get(key).dirty;
    }
    return false;
  }

  sjtu::vector<Key> get_dirty_keys() const {
    sjtu::vector<Key> dirty_keys;

    cache_items_.for_each([&dirty_keys](const Key& key, const CacheItem& item) {
      if (item.dirty) {
        dirty_keys.push_back(key);
      }
    });

    return dirty_keys;
  }

  template <typename Func>
  void for_each_dirty(Func func) {
    sjtu::vector<Key> dirty_keys = get_dirty_keys();

    for (size_t i = 0; i < dirty_keys.size(); ++i) {
      Key key = dirty_keys[i];
      if (cache_items_.contains(key)) {
        func(key, cache_items_.get(key).value);
      }
    }
  }

  bool remove(const Key& key) {
    if (!cache_items_.contains(key)) {
      return false;
    }

    if (positions_.contains(key)) {
      lru_list_.erase(positions_.get(key).position);
      positions_.remove(key);
    }

    cache_items_.remove(key);
    return true;
  }

  void clear() {
    lru_list_.clear();
    cache_items_.clear();
    positions_.clear();
  }

  void set_eviction_callback(EvictionCallback callback) {
    eviction_callback_ = callback;
  }

 private:
  void evict() {
    if (lru_list_.empty()) return;
    Key lru_key = lru_list_.back();
    lru_list_.pop_back();
    if (cache_items_.contains(lru_key) && cache_items_.get(lru_key).dirty) {
      if (eviction_callback_) {
        eviction_callback_(lru_key, cache_items_.get(lru_key).value);
      }
    }
    cache_items_.remove(lru_key);
    positions_.remove(lru_key);
  }
};

template <class Key, class Value>
class BPTCacheManager {
 private:
  LRUCache<int, Index<Key, Value>> index_cache_;
  LRUCache<int, Block<Key, Value>> block_cache_;

  MemoryRiver<Index<Key, Value>, 2>& index_file_;
  MemoryRiver<Block<Key, Value>, 2>& block_file_;

 public:
  BPTCacheManager(MemoryRiver<Index<Key, Value>, 2>& index_file,
                  MemoryRiver<Block<Key, Value>, 2>& block_file,
                  size_t index_cache_size = 1024, size_t block_cache_size = 2048)
      : index_file_(index_file),
        block_file_(block_file),
        index_cache_(index_cache_size),
        block_cache_(block_cache_size) {
    index_cache_.set_eviction_callback(
        [this](int addr, const Index<Key, Value>& index) {
          if (index_cache_.is_dirty(addr)) {
            index_file_.update(const_cast<Index<Key, Value>&>(index), addr);
          }
        });

    block_cache_.set_eviction_callback(
        [this](int addr, const Block<Key, Value>& block) {
          if (block_cache_.is_dirty(addr)) {
            block_file_.update(const_cast<Block<Key, Value>&>(block), addr);
          }
        });
  }

  void read_index(Index<Key, Value>& index, int index_addr) {
    if (index_cache_.contains(index_addr)) {
      index = index_cache_.get(index_addr);
      return;
    }
    index_file_.read(index, index_addr);
    index_cache_.put(index_addr, index, false);
  }

  void read_block(Block<Key, Value>& block, int block_addr) {
    if (block_cache_.contains(block_addr)) {
      block = block_cache_.get(block_addr);
      return;
    }
    block_file_.read(block, block_addr);
    block_cache_.put(block_addr, block, false);
  }

  int write_index(const Index<Key, Value>& index) {
    int index_addr = index_file_.write(const_cast<Index<Key, Value>&>(index));
    index_cache_.put(index_addr, index, false);
    return index_addr;
  }

  int write_block(const Block<Key, Value>& block) {
    int block_addr = block_file_.write(const_cast<Block<Key, Value>&>(block));
    block_cache_.put(block_addr, block, false);
    return block_addr;
  }

  void update_index(const Index<Key, Value>& index, int index_addr) {
    index_cache_.put(index_addr, index, true);
  }

  void update_block(const Block<Key, Value>& block, int block_addr) {
    block_cache_.put(block_addr, block, true);
  }

  void flush_cache() {
    index_cache_.for_each_dirty(
        [this](int addr, const Index<Key, Value>& index) {
          index_file_.update(const_cast<Index<Key, Value>&>(index), addr);
          index_cache_.mark_dirty(addr, false);
        });

    block_cache_.for_each_dirty(
        [this](int addr, const Block<Key, Value>& block) {
          block_file_.update(const_cast<Block<Key, Value>&>(block), addr);
          block_cache_.mark_dirty(addr, false);
        });
  }

  void clear() {
    flush_cache();
    index_cache_.clear();
    block_cache_.clear();
  }
};

}  // namespace sjtu

#endif  // BPT_CACHE_HPP