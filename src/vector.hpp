#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include <climits>
#include <cstddef>

#include "exceptions.hpp"

constexpr size_t DEFAULT_CAPACITY = 100;
namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template <typename T>
class vector {
 public:
  /**
   * TODO
   * a type for actions of the elements of a vector, and you should write
   *   a class named const_iterator with same interfaces.
   */
  /**
   * you can see RandomAccessIterator at CppReference for help.
   */
  class const_iterator;
  class iterator {
    // The following code is written for the C++ type_traits library.
    // Type traits is a C++ feature for describing certain properties of a type.
    // For instance, for an iterator, iterator::value_type is the type that the
    // iterator points to.
    // STL algorithms and containers may use these type_traits (e.g. the
    // following typedef) to work properly. In particular, without the following
    // code,
    // @code{std::sort(iter, iter1);} would not compile.
    // See these websites for more information:
    // https://en.cppreference.com/w/cpp/header/type_traits
    // About value_type:
    // https://blog.csdn.net/u014299153/article/details/72419713 About
    // iterator_category: https://en.cppreference.com/w/cpp/iterator
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::random_access_iterator_tag;

   private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
   public:
    vector<T> *container;
    size_t index;
    iterator(vector<T> *_container = nullptr, size_t _index = 0)
        : container(_container), index(_index) {}
    /**
     * return a new iterator which pointer n-next elements
     * as well as operator-
     */
    iterator operator+(const int &n) const {
      return iterator(container, index + n);
      // TODO
    }
    iterator operator-(const int &n) const {
      return iterator(container, index - n);
      // TODO
    }
    // return the distance between two iterators,
    // if these two iterators point to different vectors, throw
    // invaild_iterator.
    int operator-(const iterator &rhs) const {
      if (container != rhs.container) {
        throw invalid_iterator();
      }
      return index - rhs.index;
      // TODO
    }
    iterator &operator+=(const int &n) {
      index += n;
      return *this;
      // TODO
    }
    iterator &operator-=(const int &n) {
      index -= n;
      return *this;
      // TODO
    }
    /**
     * TODO iter++
     */
    iterator operator++(int) {
      iterator tmp = *this;
      ++index;
      return tmp;
    }
    /**
     * TODO ++iter
     */
    iterator &operator++() {
      ++index;
      return *this;
    }
    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator tmp = *this;
      --index;
      return tmp;
    }
    /**
     * TODO --iter
     */
    iterator &operator--() {
      --index;
      return *this;
    }
    /**
     * TODO *it
     */
    T &operator*() const { return (container->data_[index]); }
    /**
     * a operator to check whether two iterators are same (pointing to the same
     * memory address).
     */
    bool operator==(const iterator &rhs) const {
      return index == rhs.index && container == rhs.container;
    }
    bool operator==(const const_iterator &rhs) const {
      return index == rhs.index && container == rhs.container;
    }
    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return index != rhs.index || container != rhs.container;
    }
    bool operator!=(const const_iterator &rhs) const {
      return index != rhs.index || container != rhs.container;
    }
  };
  /**
   * TODO
   * has same function as iterator, just for a const object.
   */
  class const_iterator {
   public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

   private:
   public:
    const vector<T> *container;
    size_t index;
    const_iterator(const vector<T> *_container = nullptr, size_t _index = 0)
        : container(_container), index(_index) {}
    const T &operator*() const { return (container->data_[index]); }

    bool operator==(const const_iterator &rhs) const {
      return index == rhs.index && container == rhs.container;
    }

    bool operator!=(const const_iterator &rhs) const {
      return index != rhs.index || container != rhs.container;
    }
    const_iterator &operator++() {
      ++index;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator temp = *this;
      ++(*this);
      return temp;
    }
    const_iterator &operator--() {
      --index;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator temp = *this;
      --(*this);
      return temp;
    }
  };
  /**
   * TODO Constructs
   * At least two: default constructor, copy constructor
   */
  vector() : size_(0), capacity_(DEFAULT_CAPACITY) {
    data_ = reinterpret_cast<T *>(::operator new(sizeof(T) * capacity_));
  }
  vector(const vector &other) {
    size_ = other.size_;
    capacity_ = other.capacity_;
    data_ = reinterpret_cast<T *>(::operator new(sizeof(T) * capacity_));
    for (size_t i = 0; i < size_; ++i) {
      new (data_ + i) T(other.data_[i]);
    }
  }
  /**
   * TODO Destructor
   */
  ~vector() {
    for (size_t i = 0; i < size_; ++i) {
      data_[i].~T();
    }
    ::operator delete(data_);
  }
  /**
   * TODO Assignment operator
   */
  vector &operator=(const vector &other) {
    if (this == &other) {
      return *this;
    }
    for (size_t i = 0; i < size_; ++i) {
      data_[i].~T();
    }
    ::operator delete(data_);
    size_ = other.size_;
    capacity_ = other.capacity_;
    data_ = reinterpret_cast<T *>(::operator new(sizeof(T) * capacity_));
    for (size_t i = 0; i < size_; ++i) {
      new (data_ + i) T(other.data_[i]);
    }
    return *this;
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   */
  T &at(const size_t &pos) {
    if (pos < 0 || pos >= size_) {
      throw index_out_of_bound();
    }
    return data_[pos];
  }
  const T &at(const size_t &pos) const {
    if (pos < 0 || pos >= size_) {
      throw index_out_of_bound();
    }
    return data_[pos];
  }
  /**
   * assigns specified element with bounds checking
   * throw index_out_of_bound if pos is not in [0, size)
   * !!! Pay attentions
   *   In STL this operator does not check the boundary but I want you to do.
   */
  T &operator[](const size_t &pos) {
    if (pos < 0 || pos >= size_) {
      throw index_out_of_bound();
    }
    return data_[pos];
  }
  const T &operator[](const size_t &pos) const {
    if (pos < 0 || pos >= size_) {
      throw index_out_of_bound();
    }
    return data_[pos];
  }
  /**
   * access the first element.
   * throw container_is_empty if size == 0
   */
  const T &front() const {
    if (size_ == 0) {
      throw container_is_empty();
    }
    return data_[0];
  }
  /**
   * access the last element.
   * throw container_is_empty if size == 0
   */
  const T &back() const {
    if (size_ == 0) {
      throw container_is_empty();
    }
    return data_[size_ - 1];
  }
  /**
   * returns an iterator to the beginning.
   */
  iterator begin() { return iterator(this, 0); }
  const_iterator begin() const { return const_iterator(this, 0); }
  const_iterator cbegin() const { return const_iterator(this, 0); }
  /**
   * returns an iterator to the end.
   */
  iterator end() { return iterator(this, size_); }
  const_iterator end() const { return const_iterator(this, size_); }
  const_iterator cend() const { return const_iterator(this, size_); }
  /**
   * checks whether the container is empty
   */
  bool empty() const { return size_ == 0; }
  /**
   * returns the number of elements
   */
  size_t size() const { return size_; }
  /**
   * clears the contents
   */
  void clear() {
    for (size_t i = 0; i < size_; ++i) {
      data_[i].~T();
    }
    size_ = 0;
  }
  /**
   * inserts value before pos
   * returns an iterator pointing to the inserted value.
   */
  iterator insert(iterator pos, const T &value) {
    if (size_ == capacity_) {
      double_space();
    }
    for (size_t i = size_; i > pos.index; --i) {
      new (data_ + i) T(data_[i - 1]);
      data_[i - 1].~T();
    }
    new (data_ + pos.index) T(value);
    ++size_;
    return pos;
  }
  /**
   * inserts value at index ind.
   * after inserting, this->at(ind) == value
   * returns an iterator pointing to the inserted value.
   * throw index_out_of_bound if ind > size (in this situation ind can be size
   * because after inserting the size will increase 1.)
   */
  iterator insert(const size_t &ind, const T &value) {
    if (ind > size_) {
      throw index_out_of_bound();
    }
    if (size_ == capacity_) {
      double_space();
    }
    for (size_t i = size_; i > ind; --i) {
      new (data_ + i) T(data_[i - 1]);
      data_[i - 1].~T();
    }
    new (data_ + ind) T(value);
    ++size_;
    return iterator(this, ind);
  }
  /**
   * removes the element at pos.
   * return an iterator pointing to the following element.
   * If the iterator pos refers the last element, the end() iterator is
   * returned.
   */
  iterator erase(iterator pos) {
    if (pos.index >= size_) {
      throw index_out_of_bound();
    }
    data_[pos.index].~T();
    for (size_t i = pos.index; i < size_ - 1; ++i) {
      new (data_ + i) T(data_[i + 1]);
      data_[i + 1].~T();
    }
    --size_;
    return pos;
  }
  /**
   * removes the element with index ind.
   * return an iterator pointing to the following element.
   * throw index_out_of_bound if ind >= size
   */
  iterator erase(const size_t &ind) {
    if (ind >= size_) {
      throw index_out_of_bound();
    }
    data_[ind].~T();
    for (size_t i = ind; i < size_ - 1; ++i) {
      new (data_ + i) T(data_[i + 1]);
      data_[i + 1].~T();
    }
    --size_;
    return iterator(this, ind);
  }
  /**
   * adds an element to the end.
   */
  void push_back(const T &value) {
    if (size_ == capacity_) {
      double_space();
    }
    new (data_ + size_) T(value);
    ++size_;
  }
  /**
   * remove the last element from the end.
   * throw container_is_empty if size() == 0
   */
  void pop_back() {
    if (size_ == 0) {
      throw container_is_empty();
    }
    data_[size_ - 1].~T();
    --size_;
  }

 private:
  T *data_;
  size_t size_;
  size_t capacity_;

  void double_space() {
    T *new_data =
        reinterpret_cast<T *>(::operator new(sizeof(T) * capacity_ * 2));
    for (size_t i = 0; i < size_; ++i) {
      new (new_data + i) T(data_[i]);
      data_[i].~T();
    }
    ::operator delete(data_);
    data_ = new_data;
    capacity_ *= 2;
  }
};

}  // namespace sjtu

#endif