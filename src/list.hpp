#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP

#include <cstddef>
#include <type_traits>

#include "exceptions.hpp"

namespace sjtu {
/**
 * a data container like std::list
 * allocate random memory addresses for data and they are doubly-linked in a
 * list.
 */

template <typename T>
class list {
 protected:
  class node {
   public:
    /**
     * add data members and constructors & destructor
     */
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data_buf;
    node *prev, *next;
    bool valid;
    node() : prev(nullptr), next(nullptr), valid(false) {}
    node(const T &value, node *prev_ = nullptr, node *next_ = nullptr)
        : prev(prev_), next(next_), valid(true) {
      new (&data_buf) T(value);
    }
    ~node() {
      if (valid) {
        reinterpret_cast<T *>(&data_buf)->~T();
      }
    }
    T &data() { return *reinterpret_cast<T *>(&data_buf); }
  };

 protected:
  /**
   * add data members for linked list as protected members
   */
  node *head, *tail;
  size_t sz;

  /**
   * insert node cur before node pos
   * return the inserted node cur
   */
  node *insert(node *pos, node *cur) {
    cur->prev = pos->prev;
    cur->next = pos;
    pos->prev->next = cur;
    pos->prev = cur;
    return cur;
  }
  /**
   * remove node pos from list (no need to delete the node)
   * return the removed node pos
   */
  node *erase(node *pos) {
    pos->prev->next = pos->next;
    pos->next->prev = pos->prev;

    return pos;
  }

 public:
  class const_iterator;
  class iterator {
   private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
   public:
    node *cur;
    iterator(node *p = nullptr) : cur(p) {}
    iterator operator++(int) {
      iterator tmp = *this;
      cur = cur->next;
      return tmp;
    }
    iterator &operator++() {
      cur = cur->next;
      return *this;
    }
    iterator operator--(int) {
      iterator tmp = *this;
      cur = cur->prev;
      return tmp;
    }
    iterator &operator--() {
      cur = cur->prev;
      return *this;
    }

    /**
     * TODO *it
     * throw std::exception if iterator is invalid
     */
    T &operator*() const {
      if (cur == nullptr || cur->prev == nullptr || cur->next == nullptr)
        throw std::exception();
      return cur->data();
    }
    /**
     * TODO it->field
     * throw std::exception if iterator is invalid
     */
    T *operator->() const noexcept {
      if (cur == nullptr || cur->prev == nullptr || cur->next == nullptr)
        throw std::exception();
      return &(cur->data());
    }

    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory).
     */
    bool operator==(const iterator &rhs) const { return cur == rhs.cur; }
    bool operator==(const const_iterator &rhs) const { return cur == rhs.cur; }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const { return cur != rhs.cur; }
    bool operator!=(const const_iterator &rhs) const { return cur != rhs.cur; }
  };

  /**
   * TODO
   * has same function as iterator, just for a const object.
   * should be able to construct from an iterator.
   */
  class const_iterator {
   private:
   public:
    node *cur;
    const_iterator(node *p = nullptr) : cur(p) {}
    const_iterator(const iterator &other) : cur(other.cur) {}
    const_iterator(const const_iterator &other) : cur(other.cur) {}
    const_iterator operator++(int) {
      const_iterator tmp = *this;
      cur = cur->next;
      return tmp;
    }
    const_iterator &operator++() {
      cur = cur->next;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator tmp = *this;
      cur = cur->prev;
      return tmp;
    }
    const_iterator &operator--() {
      cur = cur->prev;
      return *this;
    }
    const T &operator*() const {
      if (cur == nullptr || cur->prev == nullptr) throw std::exception();
      return cur->data();
    }

    /**
     * TODO it->field
     * throw std::exception if iterator is invalid
     */
    const T *operator->() const noexcept {
      if (cur == nullptr || cur->prev == nullptr) throw std::exception();
      return &(cur->data());
    }

    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory).
     */
    bool operator==(const iterator &rhs) const { return cur == rhs.cur; }
    bool operator==(const const_iterator &rhs) const { return cur == rhs.cur; }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const { return cur != rhs.cur; }
    bool operator!=(const const_iterator &rhs) const { return cur != rhs.cur; }
  };

  /**
   * TODO Constructs
   * Atleast two: default constructor, copy constructor
   */
  list() {
    head = new node();
    tail = new node();
    head->next = tail;
    tail->prev = head;
    sz = 0;
  }
  list(const list &other) : list() {
    for (node *p = other.head->next; p != other.tail; p = p->next) {
      push_back(p->data());
    }
  }
  /**
   * TODO Destructor
   */
  virtual ~list() {
    clear();
    delete head;
    delete tail;
  }
  /**
   * TODO Assignment operator
   */
  list &operator=(const list &other) {
    if (this == &other) {
      return *this;
    }
    clear();
    for (node *p = other.head->next; p != other.tail; p = p->next) {
      push_back(p->data());
    }
    return *this;
  }
  /**
   * access the first / last element
   * throw container_is_empty when the container is empty.
   */
  const T &front() const {
    if (empty()) {
      throw container_is_empty();
    }
    return head->next->data();
  }
  const T &back() const {
    if (empty()) {
      throw container_is_empty();
    }
    return tail->prev->data();
  }
  /**
   * returns an iterator to the beginning.
   */
  iterator begin() { return iterator(head->next); }
  const_iterator cbegin() const { return const_iterator(head->next); }
  /**
   * returns an iterator to the end.
   */
  iterator end() { return iterator(tail); }
  const_iterator cend() const { return const_iterator(tail); }
  /**
   * checks whether the container is empty.
   */
  virtual bool empty() const { return sz == 0; }
  /**
   * returns the number of elements
   */
  virtual size_t size() const { return sz; }

  /**
   * clears the contents
   */
  virtual void clear() {
    node *cur = head->next;
    while (cur != tail) {
      node *tmp = cur;
      cur = cur->next;
      delete tmp;
    }
    head->next = tail;
    tail->prev = head;
    sz = 0;
  }
  /**
   * insert value before pos (pos may be the end() iterator)
   * return an iterator pointing to the inserted value
   * throw if the iterator is invalid
   */
  virtual iterator insert(iterator pos, const T &value) {
    if (pos.cur == nullptr || pos.cur->prev == nullptr) throw std::exception();
    sz++;
    return iterator(insert(pos.cur, new node(value)));
  }
  /**
   * remove the element at pos (the end() iterator is invalid)
   * returns an iterator pointing to the following element, if pos pointing to
   * the last element, end() will be returned. throw if the container is empty,
   * the iterator is invalid
   */
  virtual iterator erase(iterator pos) {
    if (empty()) {
      throw container_is_empty();
    }
    if (pos.cur == nullptr || pos.cur->prev == nullptr ||
        pos.cur->next == nullptr) {
      throw std::exception();
    }
    sz--;
    node *tmp = pos.cur->next;
    delete erase(pos.cur);
    return iterator(tmp);
  }
  /**
   * adds an element to the end
   */
  void push_back(const T &value) {
    sz++;
    insert(tail, new node(value));
  }
  /**
   * removes the last element
   * throw when the container is empty.
   */
  void pop_back() {
    if (empty()) {
      throw container_is_empty();
    }
    sz--;
    delete erase(tail->prev);
  }
  /**
   * inserts an element to the beginning.
   */
  void push_front(const T &value) {
    sz++;
    insert(head->next, new node(value));
  }
  /**
   * removes the first element.
   * throw when the container is empty.
   */
  void pop_front() {
    if (empty()) {
      throw container_is_empty();
    }
    sz--;
    delete erase(head->next);
  }
};

}  // namespace sjtu

#endif  // SJTU_LIST_HPP