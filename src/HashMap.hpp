#ifndef SJTU_HASH_MAP_HPP
#define SJTU_HASH_MAP_HPP

#include "vector.hpp"

constexpr size_t MAX_BUCKETS = 1193;
namespace sjtu {

template <class Key, class Value>
class HashMap {
 private:
  struct Entry {
    Key key;
    Value value;

    Entry() {}
    Entry(const Key& k, const Value& v) : key(k), value(v) {}
  };

  typedef sjtu::vector<Entry> Bucket;

  size_t hash_function(const Key& key) const {
    return static_cast<size_t>(key) % MAX_BUCKETS;
  }

  int find_in_bucket(const Bucket& bucket, const Key& key) const {
    for (size_t i = 0; i < bucket.size(); ++i) {
      if (bucket[i].key == key) {
        return i;
      }
    }
    return -1;
  }
  Bucket buckets[MAX_BUCKETS];
  size_t size_;

 public:
  HashMap() : size_(0) {}

  size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  bool contains(const Key& key) const {
    size_t idx = hash_function(key);
    return find_in_bucket(buckets[idx], key) != -1;
  }

  Value get(const Key& key) const {
    size_t idx = hash_function(key);
    int entry_idx = find_in_bucket(buckets[idx], key);

    if (entry_idx == -1) {
      throw sjtu::runtime_error();
    }

    return buckets[idx][entry_idx].value;
  }

  Value* get_ptr(const Key& key) {
    size_t idx = hash_function(key);
    int entry_idx = find_in_bucket(buckets[idx], key);

    if (entry_idx == -1) {
      return nullptr;
    }

    return &(buckets[idx][entry_idx].value);
  }

  void put(const Key& key, const Value& value) {
    size_t idx = hash_function(key);
    int entry_idx = find_in_bucket(buckets[idx], key);

    if (entry_idx != -1) {
      buckets[idx][entry_idx].value = value;
    } else {
      buckets[idx].push_back(Entry(key, value));
      size_++;
    }
  }

  bool remove(const Key& key) {
    size_t idx = hash_function(key);
    int entry_idx = find_in_bucket(buckets[idx], key);

    if (entry_idx == -1) {
      return false;
    }

    if (entry_idx < buckets[idx].size() - 1) {
      buckets[idx][entry_idx] = buckets[idx][buckets[idx].size() - 1];
    }
    buckets[idx].pop_back();
    size_--;

    return true;
  }

  void clear() {
    for (size_t i = 0; i < MAX_BUCKETS; ++i) {
      buckets[i].clear();
    }
    size_ = 0;
  }

  template <typename Func>
  void for_each(Func func) const {
    for (size_t i = 0; i < MAX_BUCKETS; ++i) {
      for (size_t j = 0; j < buckets[i].size(); ++j) {
        func(buckets[i][j].key, buckets[i][j].value);
      }
    }
  }
};

}  // namespace sjtu

#endif  // SJTU_HASH_MAP_HPP