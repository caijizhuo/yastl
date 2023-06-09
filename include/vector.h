﻿#ifndef _INCLUDE_VECTOR_H_
#define _INCLUDE_VECTOR_H_

// 这个头文件包含一个模板类 vector
// vector : 向量

// notes:
//
// 异常保证：
// yastl::vecotr<T> 满足基本异常保证，部分函数无异常保证，并对以下函数做强异常安全保证：
//   * emplace
//   * emplace_back
//   * push_back
// 当 std::is_nothrow_move_assignable<T>::value == true 时，以下函数也满足强异常保证：
//   * reserve
//   * resize
//   * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"
// #include "allocator.h"
#include "algo.h"

namespace yastl
{

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

// 模板类: vector 
// 模板参数 T 代表类型
template <class T>
class vector {
  static_assert(!std::is_same<bool, T>::value, "vector<bool> is abandoned in yastl");
public:
  // vector 的嵌套型别定义
  typedef yastl::allocator<T> allocator_type;
  typedef yastl::allocator<T> data_allocator;

  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;

  typedef value_type* iterator;
  typedef const value_type* const_iterator;
  typedef yastl::reverse_iterator<iterator> reverse_iterator;
  typedef yastl::reverse_iterator<const_iterator> const_reverse_iterator;

  allocator_type get_allocator() { return data_allocator(); }

private:
  iterator begin_;  // 表示目前使用空间的头部
  iterator end_;    // 表示目前使用空间的尾部
  iterator cap_;    // 表示目前储存空间的尾部

public:
  // 构造、复制、移动、析构函数
  vector() noexcept {
    try_init();
  }

  explicit vector(size_type n) {
    fill_init(n, value_type());
  }

  vector(size_type n, const value_type& value) {
    fill_init(n, value);
  }

  // 如果传入的参数是迭代器
  template <class Iter, typename std::enable_if<yastl::is_input_iterator<Iter>::value, int>::type = 0>
  vector(Iter first, Iter last) {
    YASTL_DEBUG(!(last < first));
    range_init(first, last);
  }

  vector(const vector& rhs) {
    range_init(rhs.begin_, rhs.end_);
  }

  // 移动构造
  vector(vector&& rhs) noexcept : begin_(rhs.begin_), end_(rhs.end_), cap_(rhs.cap_) {
    rhs.begin_ = nullptr;
    rhs.end_ = nullptr;
    rhs.cap_ = nullptr;
  }

  // 用std的initializer_list做初始化
  vector(std::initializer_list<value_type> ilist) {
    range_init(ilist.begin(), ilist.end());
  }

  // 赋值声明
  vector& operator=(const vector& rhs);
  vector& operator=(vector&& rhs) noexcept;

  vector& operator=(std::initializer_list<value_type> ilist) {
    vector tmp(ilist.begin(), ilist.end());
    swap(tmp);
    return *this;
  }

  ~vector() {
    destroy_and_recover(begin_, end_, cap_ - begin_);
    begin_ = end_ = cap_ = nullptr;
  }

public:

  // 迭代器相关操作
  // 返回第一个迭代器
  iterator begin() noexcept {
    return begin_;
  }
  const_iterator begin() const noexcept {
    return begin_;
  }
  iterator end() noexcept {
    return end_;
  }
  const_iterator end() const noexcept {
    return end_;
  }

  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }
  const_reverse_iterator rbegin()  const noexcept {
    return const_reverse_iterator(end());
  }
  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  const_iterator cbegin() const noexcept {
    return begin();
  }
  const_iterator cend() const noexcept {
    return end();
  }
  const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }
  const_reverse_iterator crend() const noexcept {
    return rend();
  }

  // 容量相关操作
  bool empty() const noexcept {
    return begin_ == end_;
  }
  size_type size() const noexcept {
    return static_cast<size_type>(end_ - begin_);
  }
  // 用溢出值做被除数
  size_type max_size() const noexcept {
    return static_cast<size_type>(-1) / sizeof(T);
  }
  size_type capacity() const noexcept {
    return static_cast<size_type>(cap_ - begin_);
  }
  void reserve(size_type n);
  void shrink_to_fit();

  // 访问元素相关操作
  reference operator[](size_type n) {
    YASTL_DEBUG(n < size());
    return *(begin_ + n);
  }
  const_reference operator[](size_type n) const {
    YASTL_DEBUG(n < size());
    return *(begin_ + n);
  }
  // 和[]返回结果相同
  reference at(size_type n) {
    THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
    return (*this)[n];
  }
  const_reference at(size_type n) const {
    THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
    return (*this)[n];
  }

  reference front() {
    YASTL_DEBUG(!empty());
    return *begin_;
  }
  const_reference front() const {
    YASTL_DEBUG(!empty());
    return *begin_;
  }
  reference back() {
    YASTL_DEBUG(!empty());
    return *(end_ - 1);
  }
  const_reference back() const {
    YASTL_DEBUG(!empty());
    return *(end_ - 1);
  }

  // data()返回第一个元素的指针
  pointer data() noexcept {
    return begin_;
  }
  const_pointer data() const noexcept {
    return begin_;
  }

  // 修改容器相关操作

  // assign
  // 赋值n个value给本vector
  void assign(size_type n, const value_type& value) {
    fill_assign(n, value);
  }
  // 把本vector赋值成[first, last)的内容
  template <class Iter, typename std::enable_if<yastl::is_input_iterator<Iter>::value, int>::type = 0>
  void assign(Iter first, Iter last) {
    YASTL_DEBUG(!(last < first));
    copy_assign(first, last, iterator_category(first));
  }
  // 用initializer_list来初始化
  void assign(std::initializer_list<value_type> il) {
    copy_assign(il.begin(), il.end(), yastl::forward_iterator_tag{});
  }

  // emplace / emplace_back

  template <class... Args>
  iterator emplace(const_iterator pos, Args&& ...args);

  template <class... Args>
  void emplace_back(Args&& ...args);

  // push_back / pop_back

  void push_back(const value_type& value);
  void push_back(value_type&& value) { // 转换成右值
    emplace_back(yastl::move(value));
  }

  void pop_back();

  // insert
  // 在pos 插入value
  iterator insert(const_iterator pos, const value_type& value);
  // 在pos 插入value 右值
  iterator insert(const_iterator pos, value_type&& value) {
    return emplace(pos, yastl::move(value)); // 复用emplace，移动构造节省成本
  }
  // 在 pos插入n个value
  iterator insert(const_iterator pos, size_type n, const value_type& value) {
    YASTL_DEBUG(pos >= begin() && pos <= end());
    return fill_insert(const_cast<iterator>(pos), n, value);
  }
  // 在pos 插入[first, last)
  template <class Iter, typename std::enable_if<yastl::is_input_iterator<Iter>::value, int>::type = 0>
  void insert(const_iterator pos, Iter first, Iter last) {
    YASTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
    copy_insert(const_cast<iterator>(pos), first, last);
  }

  // erase / clear
  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);
  // 全部清除
  void clear() {
    erase(begin(), end()); 
  }

  // resize / reverse
  void resize(size_type new_size) {
    return resize(new_size, value_type());
  }
  void resize(size_type new_size, const value_type& value);

  // 反转当前vector
  void reverse() {
    yastl::reverse(begin(), end());
  }

  // swap 当前vector和某个vector做交换
  void swap(vector& rhs) noexcept;

private:
  // helper functions

  // initialize / destroy 尝试初始化16个cap的元素
  void try_init() noexcept;
  // 尝试初始化size和cap的函数
  void init_space(size_type size, size_type cap);

  void fill_init(size_type n, const value_type& value);
  template <class Iter>
  void range_init(Iter first, Iter last);

  void destroy_and_recover(iterator first, iterator last, size_type n);

  // calculate the growth size 获得应该变成的大小数
  size_type get_new_cap(size_type add_size);

  // assign
  // 和init的区别是，那个是初始化用的，而此函数在已初始化的vector调用
  void fill_assign(size_type n, const value_type& value);

  template <class IIter>
  void copy_assign(IIter first, IIter last, input_iterator_tag);

  template <class FIter>
  void copy_assign(FIter first, FIter last, forward_iterator_tag);

  // reallocate

  template <class... Args>
  void reallocate_emplace(iterator pos, Args&& ...args);
  void reallocate_insert(iterator pos, const value_type& value);

  // insert

  iterator fill_insert(iterator pos, size_type n, const value_type& value);
  template <class IIter>
  void copy_insert(iterator pos, IIter first, IIter last);

  // shrink_to_fit

  void reinsert(size_type size);
};

/*****************************************************************************************/

// 复制赋值操作符
template <class T>
vector<T>& vector<T>::operator=(const vector& rhs) {
  std::cout << "call copy = in vector!" << std::endl;
  if (this != &rhs) {
    const auto len = rhs.size();
    if (len > capacity()) { // 需要扩容cap
      vector tmp(rhs.begin(), rhs.end()); // 构造临时对象，避免覆盖rhs
      swap(tmp);
    } else if (size() >= len) { // size就能装下
      auto i = yastl::copy(rhs.begin(), rhs.end(), begin());
      data_allocator::destroy(i, end_);
      end_ = begin_ + len;
    } else {  // 需要扩充size，并且把cap缩成和size一样，避免空间浪费
      yastl::copy(rhs.begin(), rhs.begin() + size(), begin_);
      yastl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);
      cap_ = end_ = begin_ + len;
    }
  }
  return *this;
}

// 移动赋值操作符,当rhs为右值时使用，直接占据rhs
template <class T>
vector<T>& vector<T>::operator=(vector&& rhs) noexcept {
  std::cout << "call move = in vector!" << std::endl;
  destroy_and_recover(begin_, end_, cap_ - begin_);
  begin_ = rhs.begin_;
  end_ = rhs.end_;
  cap_ = rhs.cap_;
  rhs.begin_ = nullptr;
  rhs.end_ = nullptr;
  rhs.cap_ = nullptr;
  return *this;
}

// 预留空间大小，当原容量小于要求大小时，才会重新分配
template <class T>
void vector<T>::reserve(size_type n) {
  if (capacity() < n) {
    THROW_LENGTH_ERROR_IF(n > max_size(), "n can not larger than max_size() in vector<T>::reserve(n)");
    const auto old_size = size();
    auto tmp = data_allocator::allocate(n);
    yastl::uninitialized_move(begin_, end_, tmp); // 把begin到end的元素移动到tmp开头
    data_allocator::deallocate(begin_, cap_ - begin_); // 释放begin的内存
    begin_ = tmp;
    end_ = tmp + old_size;
    cap_ = begin_ + n;
  }
}

// 放弃多余的容量
template <class T>
void vector<T>::shrink_to_fit() {
  if (end_ < cap_) {
    reinsert(size());
  }
}

// 在 pos 位置就地构造元素，避免额外的复制或移动开销
template <class T>
template <class ...Args>
typename vector<T>::iterator
vector<T>::emplace(const_iterator pos, Args&& ...args) {
  YASTL_DEBUG(pos >= begin() && pos <= end());
  iterator xpos = const_cast<iterator>(pos);
  const size_type n = xpos - begin_;
  if (end_ != cap_ && xpos == end_) { // 在最后插入的，但是没到cap_
    data_allocator::construct(yastl::address_of(*end_), yastl::forward<Args>(args)...); // 直接在end_的地址构造一个元素
    ++end_;
  } else if (end_ != cap_) { // 不是在最后插入的，需要把pos后面的往后都挪一个位置
    auto new_end = end_;
    data_allocator::construct(yastl::address_of(*end_), *(end_ - 1)); // 在end的位置构建一个end前一个元素
    ++new_end;
    yastl::copy_backward(xpos, end_ - 1, end_); // 把[pos, end - 1)全往后挪一个位置
    *xpos = value_type(yastl::forward<Args>(args)...);
    end_ = new_end;
  } else {   // end_ == cap_ 需要重新开一片空间给xpos
    reallocate_emplace(xpos, yastl::forward<Args>(args)...);
  }
  return begin() + n; // 返回插入的位置
}

// 在尾部就地构造元素，避免额外的复制或移动开销
template <class T>
template <class ...Args>
void vector<T>::emplace_back(Args&& ...args) {
  if (end_ < cap_) { // 空间还有剩余，直接构造
    data_allocator::construct(yastl::address_of(*end_), yastl::forward<Args>(args)...); // 完美转发
    ++end_;
  } else {
    reallocate_emplace(end_, yastl::forward<Args>(args)...); // 重新分空间构造
  }
}

// 在尾部插入元素
template <class T>
void vector<T>::push_back(const value_type& value) {
  if (end_ != cap_) {
    data_allocator::construct(yastl::address_of(*end_), value); // 拷贝构造
    ++end_;
  } else {
    reallocate_insert(end_, value);
  }
}

// 弹出尾部元素
template <class T>
void vector<T>::pop_back() {
  YASTL_DEBUG(!empty());
  data_allocator::destroy(end_ - 1); // 析构最后一个元素
  --end_;
}

// 在 pos 处插入元素，拷贝构造的方式
template <class T>
typename vector<T>::iterator vector<T>::insert(const_iterator pos, const value_type& value) {
  YASTL_DEBUG(pos >= begin() && pos <= end());
  iterator xpos = const_cast<iterator>(pos);
  const size_type n = pos - begin_;
  if (end_ != cap_ && xpos == end_) { // 没超过容量并且是插入在最后
    data_allocator::construct(yastl::address_of(*end_), value); // 在end_地址原地拷贝构造
    ++end_;
  } else if (end_ != cap_) { // 没超过容量，但是插入在中间
    auto new_end = end_;
    data_allocator::construct(yastl::address_of(*end_), *(end_ - 1)); // 先把最后一个元素往后挪一个
    ++new_end;
    auto value_copy = value;  // 避免元素因以下复制操作而被改变
    yastl::copy_backward(xpos, end_ - 1, end_); // 把[pos, end_ - 1]的往后挪一个
   *xpos = yastl::move(value_copy); // 移动构造这个值
    end_ = new_end;
  } else { // 超过容量了 重新分配
    reallocate_insert(xpos, value); 
  }
  return begin_ + n; // 返回插入点的迭代器
}

// 删除 pos 位置上的元素
template <class T>
typename vector<T>::iterator
vector<T>::erase(const_iterator pos) {
  YASTL_DEBUG(pos >= begin() && pos < end());
  iterator xpos = begin_ + (pos - begin());
  yastl::move(xpos + 1, end_, xpos); // 把[pos + 1, end)移动到[pos, end - 1)
  data_allocator::destroy(end_ - 1); // 析构最后一个元素
  --end_;
  return xpos;
}

// 删除[first, last)上的元素
template <class T>
typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last) {
  YASTL_DEBUG(first >= begin() && last <= end() && !(last < first));
  const auto n = first - begin();
  iterator r = begin_ + (first - begin()); // 为了构造一个非const的值供下面函数使用
  // data_allocator::destroy(yastl::move(end_ - n, end_, r), end_); // 后面的元素往前移动(last - first)个
  data_allocator::destroy(yastl::move(r + (last - first), end_, r), end_); // 后面的元素往前移动(last - first)个
  end_ = end_ - (last - first);
  return begin_ + n;
}

// 重置容器大小,如果小于当前size则截断，大于当前size则填充value
template <class T>
void vector<T>::resize(size_type new_size, const value_type& value) {
  if (new_size < size()) {
    erase(begin() + new_size, end()); // 多的去除
  } else {
    insert(end(), new_size - size(), value); // 在尾部插上新值
  }
}

// 将调用的vector与right hand side这个vector 交换
template <class T>
void vector<T>::swap(vector<T>& rhs) noexcept {
  if (this != &rhs) {
    yastl::swap(begin_, rhs.begin_);
    yastl::swap(end_, rhs.end_);
    yastl::swap(cap_, rhs.cap_);
  }
}

/*****************************************************************************************/
// helper function

// try_init 函数，若分配失败则忽略，不抛出异常。尝试初始化16个cap的元素
template <class T>
void vector<T>::try_init() noexcept {
  try {
    begin_ = data_allocator::allocate(16);
    end_ = begin_;
    cap_ = begin_ + 16;
  } catch (...) {
    begin_ = nullptr;
    end_ = nullptr;
    cap_ = nullptr;
  }
}

// init_space 函数，尝试初始化size和cap的函数
template <class T>
void vector<T>::init_space(size_type size, size_type cap) {
  try {
    begin_ = data_allocator::allocate(cap);
    end_ = begin_ + size;
    cap_ = begin_ + cap;
  } catch (...) {
    begin_ = nullptr;
    end_ = nullptr;
    cap_ = nullptr;
    throw;
  }
}

// fill_init 函数, 初始化n个value
template <class T>
void vector<T>::fill_init(size_type n, const value_type& value) {
  const size_type init_size = yastl::max(static_cast<size_type>(16), n);
  init_space(n, init_size);
  yastl::uninitialized_fill_n(begin_, n, value);
}

// range_init 函数,用[first, last)来拷贝构造初始化当前vector
template <class T>
template <class Iter>
void vector<T>::range_init(Iter first, Iter last) {
  const size_type init_size = yastl::max(static_cast<size_type>(last - first),
                                         static_cast<size_type>(16));
  init_space(static_cast<size_type>(last - first), init_size);
  yastl::uninitialized_copy(first, last, begin_);
}

// destroy_and_recover 函数 为空间内每个元素调用析构函数并释放内存空间
template <class T>
void vector<T>::
destroy_and_recover(iterator first, iterator last, size_type n) {
  data_allocator::destroy(first, last); // 先调用析构函数
  data_allocator::deallocate(first, n); // 再释放内存空间
}

// get_new_cap 函数,给出你需要的大小，返回实际应该重新分配的大小
template <class T>
typename vector<T>::size_type vector<T>::get_new_cap(size_type add_size) {
  const auto old_size = capacity();
  THROW_LENGTH_ERROR_IF(old_size > max_size() - add_size,
                        "vector<T>'s size too big");
  if (old_size > max_size() - old_size / 2) {   // 之前的大小比max的2/3还大
    return old_size + add_size > max_size() - 16
      ? old_size + add_size : old_size + add_size + 16;
  }
  const size_type new_size = old_size == 0
    ? yastl::max(add_size, static_cast<size_type>(16))
    : yastl::max(old_size + old_size / 2, old_size + add_size);  // 扩充后应该的大小，1.5倍或者add_size
  return new_size;
}

// fill_assign 函数，填充n个为value的值
template <class T>
void vector<T>::fill_assign(size_type n, const value_type& value) {
  if (n > capacity()) { // 如果n个数量比现在的容量大，就重新开一个，之后和现在的vector交换
    vector tmp(n, value);
    swap(tmp);
  } else if (n > size()) { // 只是比现在的大，但是没超过容量,就强行改
    yastl::fill(begin(), end(), value);
    end_ = yastl::uninitialized_fill_n(end_, n - size(), value);
  } else { // 否则，从begin填充n个，之后把后面的删了
    erase(yastl::fill_n(begin_, n, value), end_);
  }
}

// copy_assign 函数，拷贝构造[first, last)范围内元素到当前vector
template <class T>
template <class IIter>
void vector<T>::copy_assign(IIter first, IIter last, input_iterator_tag) {
  auto cur = begin_;
  for (; first != last && cur != end_; ++first, ++cur) { // 当前不到末尾并且目标也没到尾部
    *cur = *first; // input iterator tag 顺序读取
  }
  if (first == last) { // 目标先到尾部，把当前到最后都给删了
    erase(cur, end_);
  } else {
    insert(end_, first, last); // 当前先到尾部，后面批量插入
  }
}

// 用 [first, last) 为容器赋值
template <class T>
template <class FIter>
void vector<T>::copy_assign(FIter first, FIter last, forward_iterator_tag) {
  const size_type len = yastl::distance(first, last);
  if (len > capacity()) { // 目标长度大于当前cap
    vector tmp(first, last); // 新建一个vector，不要影响之前的
    swap(tmp);
  } else if (size() >= len) { // 当前size已经可以容纳
    auto new_end = yastl::copy(first, last, begin_);
    data_allocator::destroy(new_end, end_);
    end_ = new_end;
  } else { // size容不下但是cap可以
    auto mid = first;
    yastl::advance(mid, size());
    yastl::copy(first, mid, begin_);
    auto new_end = yastl::uninitialized_copy(mid, last, end_);
    end_ = new_end;
  }
}

// 重新分配空间并在 pos 处就地构造元素
template <class T>
template <class ...Args>
void vector<T>::reallocate_emplace(iterator pos, Args&& ...args) {
  const auto new_size = get_new_cap(1); // 获得新的大小，不一定是1，只是语义上插入一个元素
  auto new_begin = data_allocator::allocate(new_size);
  auto new_end = new_begin;
  try {
    new_end = yastl::uninitialized_move(begin_, pos, new_begin); // 先把[begin, pos)数据移动过去
    data_allocator::construct(yastl::address_of(*new_end), yastl::forward<Args>(args)...); // 在最后调用移动构造函数
    ++new_end;
    new_end = yastl::uninitialized_move(pos, end_, new_end); //再把[pos, end)的移过去
  } catch (...) {
    data_allocator::deallocate(new_begin, new_size);
    throw;
  }
  destroy_and_recover(begin_, end_, cap_ - begin_); // 释放原先的空间
  begin_ = new_begin;
  end_ = new_end;
  cap_ = new_begin + new_size;
}

// 重新分配空间并在 pos 处插入元素
template <class T>
void vector<T>::reallocate_insert(iterator pos, const value_type& value) {
  const auto new_size = get_new_cap(1);
  auto new_begin = data_allocator::allocate(new_size);
  auto new_end = new_begin;
  const value_type& value_copy = value;
  try
  {
    new_end = yastl::uninitialized_move(begin_, pos, new_begin);
    data_allocator::construct(yastl::address_of(*new_end), value_copy);
    ++new_end;
    new_end = yastl::uninitialized_move(pos, end_, new_end);
  }
  catch (...)
  {
    data_allocator::deallocate(new_begin, new_size);
    throw;
  }
  destroy_and_recover(begin_, end_, cap_ - begin_);
  begin_ = new_begin;
  end_ = new_end;
  cap_ = new_begin + new_size;
}

// fill_insert 函数，在pos位置插入n个value
template <class T>
typename vector<T>::iterator vector<T>::fill_insert(iterator pos, size_type n, const value_type& value) {
  if (n == 0) {
    return pos;
  }
  const size_type xpos = pos - begin_;
  const value_type value_copy = value;  // 避免被覆盖
  if (static_cast<size_type>(cap_ - end_) >= n) { // 如果备用空间大于等于增加的空间
    const size_type after_elems = end_ - pos; // pos后面元素个数
    auto old_end = end_;
    if (after_elems > n) { // [pos, end)的空间够容纳插入的n个元素
      yastl::uninitialized_copy(end_ - n, end_, end_); // 把[end - n, end)先移动到[end, end + n)
      end_ += n;
      yastl::move_backward(pos, old_end - n, old_end); // 把[pos, end - n)移动到[x, end)
      yastl::uninitialized_fill_n(pos, n, value_copy); // 填充n个
    } else { // [pos, end)剩余空间不够，但是[pos, cap)够
      end_ = yastl::uninitialized_fill_n(end_, n - after_elems, value_copy); // 把[end, n - end)填上value
      end_ = yastl::uninitialized_move(pos, old_end, end_); // end后的元素后移到end后
      yastl::uninitialized_fill_n(pos, after_elems, value_copy); // 把[pos, end)填上value
    }
  } else { // 如果备用空间不足
    const auto new_size = get_new_cap(n);
    auto new_begin = data_allocator::allocate(new_size); // 重新开一块内存
    auto new_end = new_begin;
    try {
      new_end = yastl::uninitialized_move(begin_, pos, new_begin); // [begin, pos) 到 new_begin
      new_end = yastl::uninitialized_fill_n(new_end, n, value); // 插入n个value
      new_end = yastl::uninitialized_move(pos, end_, new_end);  // [pos, end) 继续移动
    } catch (...) {
      destroy_and_recover(new_begin, new_end, new_size);
      throw;
    }
    data_allocator::deallocate(begin_, cap_ - begin_); // 原来的内存释放
    begin_ = new_begin;
    end_ = new_end;
    cap_ = begin_ + new_size;
  }
  return begin_ + xpos;
}

// copy_insert 函数 在pos 插入[first, last)数据，拷贝构造
template <class T>
template <class IIter>
void vector<T>::copy_insert(iterator pos, IIter first, IIter last) {
  if (first == last) {
    return;
  }
  const auto n = yastl::distance(first, last);
  if ((cap_ - end_) >= n) { // 如果备用空间大小足够
    const auto after_elems = end_ - pos;
    auto old_end = end_;
    if (after_elems > n) { // [pos, end)不需要全挪出这个区间
      end_ = yastl::uninitialized_copy(end_ - n, end_, end_); // 往后挪
      yastl::move_backward(pos, old_end - n, old_end);
      yastl::uninitialized_copy(first, last, pos); // 构造新的
    } else {  // [pos, end)需要全挪出这个区间
      auto mid = first;
      yastl::advance(mid, after_elems); // 为什么使用advance，因为pos和first可能是两种不同的迭代器
      end_ = yastl::uninitialized_copy(mid, last, end_); // 因为是拷贝构造，所以用copy
      end_ = yastl::uninitialized_move(pos, old_end, end_); // 自己内部元素移动用move快
      yastl::uninitialized_copy(first, mid, pos); // 因为是拷贝构造，所以用copy
    }
  } else { // 备用空间不足
    const auto new_size = get_new_cap(n);
    auto new_begin = data_allocator::allocate(new_size);
    auto new_end = new_begin;
    try {
      new_end = yastl::uninitialized_move(begin_, pos, new_begin);
      new_end = yastl::uninitialized_copy(first, last, new_end);
      new_end = yastl::uninitialized_move(pos, end_, new_end);
    } catch (...) {
      destroy_and_recover(new_begin, new_end, new_size);
      throw;
    }
    data_allocator::deallocate(begin_, cap_ - begin_);
    begin_ = new_begin;
    end_ = new_end;
    cap_ = begin_ + new_size;
  }
}

// reinsert 函数 新开一块size大小的空间，并把当前内容挪过去
template <class T>
void vector<T>::reinsert(size_type size) {
  auto new_begin = data_allocator::allocate(size);
  try {
    yastl::uninitialized_move(begin_, end_, new_begin);
  } catch (...) {
    data_allocator::deallocate(new_begin, size);
    throw;
  }
  data_allocator::deallocate(begin_, cap_ - begin_);
  begin_ = new_begin;
  end_ = begin_ + size;
  cap_ = begin_ + size;
}

/*****************************************************************************************/
// 重载比较操作符

template <class T>
bool operator==(const vector<T>& lhs, const vector<T>& rhs) {
  return lhs.size() == rhs.size() && yastl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <class T>
bool operator<(const vector<T>& lhs, const vector<T>& rhs) {
  return yastl::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), lhs.end());
}

// 判断内容是否相等
template <class T>
bool operator!=(const vector<T>& lhs, const vector<T>& rhs) {
  return !(lhs == rhs);
}

template <class T>
bool operator>(const vector<T>& lhs, const vector<T>& rhs) {
  return rhs < lhs;
}

template <class T>
bool operator<=(const vector<T>& lhs, const vector<T>& rhs) {
  return !(rhs < lhs);
}

template <class T>
bool operator>=(const vector<T>& lhs, const vector<T>& rhs) {
  return !(lhs < rhs);
}

// 重载 yastl 的 swap
template <class T>
void swap(vector<T>& lhs, vector<T>& rhs) {
  lhs.swap(rhs);
}

} // namespace yastl
#endif // _INCLUDE_VECTOR_H_

