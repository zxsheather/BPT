#pragma once
#include <string>

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

  bool operator!=(const Key_Value &other) const { return !(*this == other); }

  bool operator<=(const Key_Value &other) const {
    return *this < other || *this == other;
  }

  bool operator>=(const Key_Value &other) const {
    return *this > other || *this == other;
  }
};

constexpr size_t DEFAULT_ORDER = 55;
constexpr size_t DEFAULT_LEAF_SIZE = 55;

// Increment the size of keys to facilitate split
template <class Key, class Value>
struct Index {
  int children[DEFAULT_ORDER + 1];
  Key_Value<Key, Value> keys[DEFAULT_ORDER];
  size_t size;

  Index() : size(0) {}

  Index(const Index &other) {
    size = other.size;
    for (size_t i = 0; i < size; ++i) {
      keys[i] = other.keys[i];
      children[i] = other.children[i];
    }
    children[size] = other.children[size];
  }
  Index &operator=(const Index &other) {
    if (this != &other) {
      size = other.size;
      for (size_t i = 0; i < size; ++i) {
        keys[i] = other.keys[i];
        children[i] = other.children[i];
      }
      children[size] = other.children[size];
    }
    return *this;
  }
  Index(Index &&other) noexcept {
    size = other.size;
    for (size_t i = 0; i < size; ++i) {
      keys[i] = other.keys[i];
      children[i] = other.children[i];
    }
    children[size] = other.children[size];
  }
  Index &operator=(Index &&other) noexcept {
    if (this != &other) {
      size = other.size;
      for (size_t i = 0; i < size; ++i) {
        keys[i] = other.keys[i];
        children[i] = other.children[i];
      }
      children[size] = other.children[size];
    }
    return *this;
  }
};

template <class Key, class Value>
struct Block {
  int next;
  Key_Value<Key, Value> data[DEFAULT_LEAF_SIZE + 1];
  size_t size;

  Block() : next(-1), size(0) {}

  Block(const Block &other) {
    next = other.next;
    size = other.size;
    for (size_t i = 0; i < size; ++i) {
      data[i] = other.data[i];
    }
  }
  Block &operator=(const Block &other) {
    if (this != &other) {
      next = other.next;
      size = other.size;
      for (size_t i = 0; i < size; ++i) {
        data[i] = other.data[i];
      }
    }
    return *this;
  }
  Block(Block &&other) noexcept {
    next = other.next;
    size = other.size;
    for (size_t i = 0; i < size; ++i) {
      data[i] = other.data[i];
    }
  }
  Block &operator=(Block &&other) noexcept {
    if (this != &other) {
      next = other.next;
      size = other.size;
      for (size_t i = 0; i < size; ++i) {
        data[i] = other.data[i];
      }
    }
    return *this;
  }
};