#ifndef _INCLUDE_LIST_H_
#define _INCLUDE_LIST_H_

// 这个头文件包含了一个模板类 list
// list : 双向链表

// notes:
//
// 异常保证：
// yastl::list<T> 满足基本异常保证，部分函数无异常保证，并对以下等函数做强异常安全保证：
//   * emplace_front
//   * emplace_back
//   * emplace
//   * push_front
//   * push_back
//   * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "util.h"
#include "exceptdef.h"

namespace yastl {

template <class T>
struct list_node_base;

template <class T>
struct list_node;

// list 的节点结构
// =========
// | prev  |
// =========
// | next  |
// =========
// 专注于连接关系
template <class T>
struct list_node_base {
  typedef list_node_base<T>* base_ptr;
  typedef list_node<T>* node_ptr;

  base_ptr prev;  // 前一节点
  base_ptr next;  // 下一节点

  list_node_base() = default;
  // 转换成node类型的指针
  node_ptr as_node() {
    return static_cast<node_ptr>(self());
  }

  // prev = next = self() 让这个节点脱离出来
  void unlink() {
    prev = next = self();
  }

  // 返回自己的指针
  base_ptr self() {
    return static_cast<base_ptr>(this);
  }
};

// =========
// | prev  |
// =========
// | next  |
// =========
// | value |
// =========
// 除了数据以外，由于继承，附带连接关系
template <class T>
struct list_node : public list_node_base<T> {
  typedef list_node_base<T>* base_ptr;
  typedef list_node<T>* node_ptr;

  T value;  // 数据域

  list_node() = default;
  // 拷贝构造
  list_node(const T& v) : value(v) {}
  // 移动构造
  list_node(T&& v) : value(yastl::move(v)) {}
  // 转换成base类型的指针
  base_ptr as_base() {
    return static_cast<base_ptr>(this);
  }
  // 返回自己的指针
  node_ptr self() {
    return static_cast<node_ptr>(this);
  }
};

// list 的迭代器设计,链表不像vector，需要自己设计迭代器
template <class T>
struct list_iterator : public yastl::iterator<yastl::bidirectional_iterator_tag, T> {
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;
  typedef list_node_base<T>* base_ptr;
  typedef list_node<T>* node_ptr;

  // 当前迭代器类型
  typedef list_iterator<T> self;

  base_ptr node_;  // 指向当前节点

  // 构造函数，给当前节点指针赋值
  list_iterator() = default;
  list_iterator(base_ptr x) : node_(x) {}
  list_iterator(node_ptr x) : node_(x->as_base()) {}
  // 拷贝构造
  list_iterator(const list_iterator& rhs) : node_(rhs.node_) {}

  // 重载操作符
  reference operator*() const {
    return node_->as_node()->value;
  }
  // 返回value地址
  pointer operator->() const {
    return &(operator*());
  }
  // 返回下一个节点的引用
  self& operator++() {
    YASTL_DEBUG(node_ != nullptr);
    node_ = node_->next;
    return *this;
  }
  // 返回下一个节点的拷贝
  self operator++(int) {
    self tmp = *this;
    ++*this;
    return tmp;
  }
  // 返回上一个节点的引用
  self& operator--() {
    YASTL_DEBUG(node_ != nullptr);
    node_ = node_->prev;
    return *this;
  }
  // 返回上一个节点的拷贝
  self operator--(int) {
    self tmp = *this;
    --*this;
    return tmp;
  }

  // 重载比较操作符
  bool operator==(const self& rhs) const {
    return node_ == rhs.node_;
  }
  bool operator!=(const self& rhs) const {
    return node_ != rhs.node_;
  }
};

template <class T>
struct list_const_iterator : public iterator<bidirectional_iterator_tag, T> {
  typedef T value_type;
  typedef const T* pointer;
  typedef const T& reference;
  typedef list_node_base<T>* base_ptr;
  typedef list_node<T>* node_ptr;
  typedef list_const_iterator<T> self;

  base_ptr node_;

  list_const_iterator() = default;
  list_const_iterator(base_ptr x) : node_(x) {}
  list_const_iterator(node_ptr x) : node_(x->as_base()) {}
  list_const_iterator(const list_iterator<T>& rhs) : node_(rhs.node_) {}
  list_const_iterator(const list_const_iterator& rhs) : node_(rhs.node_) {}

  reference operator*() const {
    return node_->as_node()->value;
  }
  pointer operator->() const {
    return &(operator*());
  }

  self& operator++() {
    YASTL_DEBUG(node_ != nullptr);
    node_ = node_->next;
    return *this;
  }
  self operator++(int) {
    self tmp = *this;
    ++*this;
    return tmp;
  }
  self& operator--() {
    YASTL_DEBUG(node_ != nullptr);
    node_ = node_->prev;
    return *this;
  }
  self operator--(int) {
    self tmp = *this;
    --*this;
    return tmp;
  }

  // 重载比较操作符
  bool operator==(const self& rhs) const {
    return node_ == rhs.node_;
  }
  bool operator!=(const self& rhs) const {
    return node_ != rhs.node_;
  }
};

// 模板类: list
// 模板参数 T 代表数据类型
template <class T>
class list {
public:
  // list 的嵌套型别定义
  typedef yastl::allocator<T> allocator_type;
  typedef yastl::allocator<T> data_allocator;
  typedef yastl::allocator<list_node_base<T>> base_allocator;
  typedef yastl::allocator<list_node<T>> node_allocator;

  typedef typename allocator_type::value_type value_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;

  typedef list_iterator<T> iterator;
  typedef list_const_iterator<T> const_iterator;
  typedef yastl::reverse_iterator<iterator> reverse_iterator;
  typedef yastl::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef list_node_base<T>* base_ptr;
  typedef list_node<T>* node_ptr;

  allocator_type get_allocator() {
    return node_allocator();
  }

private:
  // 指向末尾节点,这个节点不放值，为最后一个存放值得下一个节点
  base_ptr node_;
  // 大小
  size_type size_;

public:
  // 构造、复制、移动、析构函数
  // 构造一个空的容器
  list() {
    fill_init(0, value_type());
  }
  // 初始化n个列表
  explicit list(size_type n) {
    fill_init(n, value_type());
  }
  // 初始化n个value的列表
  list(size_type n, const T& value) {
    fill_init(n, value);
  }
  // 用迭代器初始化list
  template <class Iter, typename std::enable_if<yastl::is_input_iterator<Iter>::value, int>::type = 0>
  list(Iter first, Iter last) {
    copy_init(first, last);
  }

  list(std::initializer_list<T> ilist) {
    copy_init(ilist.begin(), ilist.end());
  }
  // 拷贝构造函数
  list(const list& rhs) {
    copy_init(rhs.cbegin(), rhs.cend());
  }
  // 移动构造，直接把rhs的内容拿来用，并防止double free
  list(list&& rhs) noexcept : node_(rhs.node_), size_(rhs.size_) {
    rhs.node_ = nullptr;
    rhs.size_ = 0;
  }
  // 赋值运算
  list& operator=(const list& rhs) {
    if (this != &rhs) {
      assign(rhs.begin(), rhs.end());
    }
    return *this;
  }

  list& operator=(list&& rhs) noexcept {
    clear();
    splice(end(), rhs); // 右值赋值，直接拼接
    return *this;
  }

  list& operator=(std::initializer_list<T> ilist) {
    list tmp(ilist.begin(), ilist.end());
    swap(tmp); // 和临时对象交换赋值
    return *this;
  }
  // 析构，在末尾节点指针不为空的情况才释放
  ~list() {
    if (node_) {
      clear();
      base_allocator::deallocate(node_);
      node_ = nullptr;
      size_ = 0;
    }
  }

public:
  // 迭代器相关操作
  // node_指向的是end，环形链表，所以是node->next
  iterator begin() noexcept {
    return node_->next;
  }
  const_iterator begin() const noexcept {
    return node_->next;
  }
  iterator end() noexcept {
    return node_;
  }
  const_iterator end() const noexcept {
    return node_;
  }

  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }
  const_reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }
  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }
  const_reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
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
    return node_->next == node_;
  }

  size_type size() const noexcept {
    return size_;
  }
  // 用size_type的最大值
  size_type max_size() const noexcept {
    return static_cast<size_type>(-1);
  }

  // 访问元素相关操作
  // 返回第一个元素的引用
  reference front() {
    YASTL_DEBUG(!empty());
    return *begin();
  }

  const_reference front() const {
    YASTL_DEBUG(!empty()); 
    return *begin(); 
  }

  reference back() {
    YASTL_DEBUG(!empty());
    return *(--end());
  }

  const_reference back() const {
    YASTL_DEBUG(!empty()); 
    return *(--end());
  }

  // 调整容器相关操作

  // assign
  // 把n个元素赋值为value
  void assign(size_type n, const value_type& value) {
    fill_assign(n, value);
  }
  // 复制[first, last)为当前list赋值
  template <class Iter, typename std::enable_if<yastl::is_input_iterator<Iter>::value, int>::type = 0>
  void assign(Iter first, Iter last) {
    copy_assign(first, last);
  }

  void assign(std::initializer_list<T> ilist) {
    copy_assign(ilist.begin(), ilist.end());
  }

  // emplace_front / emplace_back / emplace
  // 把节点插在前面
  template <class ...Args>
  void emplace_front(Args&& ...args) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(yastl::forward<Args>(args)...);
    link_nodes_at_front(link_node->as_base(), link_node->as_base());
    ++size_;
  }
  // 把节点插在后面
  template <class ...Args>
  void emplace_back(Args&& ...args) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(yastl::forward<Args>(args)...);
    link_nodes_at_back(link_node->as_base(), link_node->as_base());
    ++size_;
  }
  // 节点插在pos
  template <class ...Args>
  iterator emplace(const_iterator pos, Args&& ...args) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(yastl::forward<Args>(args)...);
    link_nodes(pos.node_, link_node->as_base(), link_node->as_base());
    ++size_;
    return iterator(link_node);
  }

  // insert
  // 对于链表来说和emplace没区别
  iterator insert(const_iterator pos, const value_type& value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(value);
    ++size_;
    return link_iter_node(pos, link_node->as_base());
  }

  iterator insert(const_iterator pos, value_type&& value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(yastl::move(value));
    ++size_;
    return link_iter_node(pos, link_node->as_base());
  }

  iterator insert(const_iterator pos, size_type n, const value_type& value) { 
    THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
    return fill_insert(pos, n, value); 
  }

  template <class Iter, typename std::enable_if<yastl::is_input_iterator<Iter>::value, int>::type = 0>
  iterator insert(const_iterator pos, Iter first, Iter last) { 
    size_type n = yastl::distance(first, last);
    THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
    return copy_insert(pos, n, first); 
  }

  // push_front / push_back
  // 插入在前面
  void push_front(const value_type& value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(value);
    link_nodes_at_front(link_node->as_base(), link_node->as_base());
    ++size_;
  }

  void push_front(value_type&& value) {
    emplace_front(yastl::move(value));
  }

  void push_back(const value_type& value) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
    auto link_node = create_node(value);
    link_nodes_at_back(link_node->as_base(), link_node->as_base());
    ++size_;
  }

  void push_back(value_type&& value) {
    emplace_back(yastl::move(value));
  }

  // pop_front / pop_back

  void pop_front() {
    YASTL_DEBUG(!empty());
    auto begin_node = node_->next; // 开头节点
    unlink_nodes(begin_node, begin_node);
    destroy_node(begin_node->as_node());
    --size_;
  }

  void pop_back() { 
    YASTL_DEBUG(!empty());
    auto n = node_->prev; // end不放值，是end前一个
    unlink_nodes(n, n);
    destroy_node(n->as_node());
    --size_;
  }

  // erase / clear

  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);

  void clear();

  // resize
  // 重新分配大小
  void resize(size_type new_size) {
    resize(new_size, value_type()); // 调用value_type及list的构造函数，默认值是0
  }
  // 重新分配大小，多余的设置为value
  void resize(size_type new_size, const value_type& value);
  // 与rhs这个list交换
  void swap(list& rhs) noexcept {
    yastl::swap(node_, rhs.node_);
    yastl::swap(size_, rhs.size_);
  }

  // list 相关操作

  void splice(const_iterator pos, list& other);
  void splice(const_iterator pos, list& other, const_iterator it);
  void splice(const_iterator pos, list& other, const_iterator first, const_iterator last);
  // 删除特定值的节点
  void remove(const value_type& value) {
    remove_if([&](const value_type& v) {
      return v == value;
    });
  }
  template <class UnaryPredicate>
  void remove_if(UnaryPredicate pred);

  // 删除连续且相等的node
  void unique() {
    unique(yastl::equal_to<T>());
  }
  template <class BinaryPredicate>
  void unique(BinaryPredicate pred);

  // 合并两个有序链表
  void merge(list& x) {
    merge(x, yastl::less<T>());
  }
  template <class Compare>
  void merge(list& x, Compare comp);

  void sort() {
    list_sort(begin(), end(), size(), yastl::less<T>());
  }
  template <class Compared>
  void sort(Compared comp) {
    list_sort(begin(), end(), size(), comp);
  }

  void reverse();

private:
  // helper functions

  // create / destroy node
  template <class ...Args>
  node_ptr create_node(Args&& ...agrs);
  void     destroy_node(node_ptr p);

  // initialize
  void      fill_init(size_type n, const value_type& value);
  template <class Iter>
  void      copy_init(Iter first, Iter last);

  // link / unlink
  iterator  link_iter_node(const_iterator pos, base_ptr node);
  void      link_nodes(base_ptr p, base_ptr first, base_ptr last);
  void      link_nodes_at_front(base_ptr first, base_ptr last);
  void      link_nodes_at_back(base_ptr first, base_ptr last);
  void      unlink_nodes(base_ptr f, base_ptr l);

  // assign
  void      fill_assign(size_type n, const value_type& value);
  template <class Iter>
  void      copy_assign(Iter first, Iter last);

  // insert
  iterator  fill_insert(const_iterator pos, size_type n, const value_type& value);
  template <class Iter>
  iterator  copy_insert(const_iterator pos, size_type n, Iter first);

  // sort
  template <class Compared>
  iterator  list_sort(iterator first, iterator last, size_type n, Compared comp);

}; // list end

/*****************************************************************************************/

// 删除 pos 处的元素
template <class T>
typename list<T>::iterator 
list<T>::erase(const_iterator pos)
{
  YASTL_DEBUG(pos != cend());
  auto n = pos.node_;
  auto next = n->next;
  unlink_nodes(n, n);
  destroy_node(n->as_node());
  --size_;
  return iterator(next);
}

// 删除 [first, last) 内的元素
template <class T>
typename list<T>::iterator 
list<T>::erase(const_iterator first, const_iterator last)
{
  if (first != last)
  {
    unlink_nodes(first.node_, last.node_->prev);
    while (first != last)
    {
      auto cur = first.node_;
      ++first;
      destroy_node(cur->as_node());
      --size_;
    }
  }
  return iterator(last.node_);
}

// 清空 list
template <class T>
void list<T>::clear()
{
  if (size_ != 0)
  {
    auto cur = node_->next;
    for (base_ptr next = cur->next; cur != node_; cur = next, next = cur->next)
    {
      destroy_node(cur->as_node());
    }
    node_->unlink();
    size_ = 0;
  }
}

// 重置容器大小
template <class T>
void list<T>::resize(size_type new_size, const value_type& value) {
  auto i = begin();
  size_type len = 0;
  while (i != end() && len < new_size) {
    ++i;
    ++len;
  }
  if (len == new_size) { // new_size < 当前大小，后面丢弃
    erase(i, node_);
  } else { // new_size > 当前大小，后面的全补value
    insert(node_, new_size - len, value);
  }
}

// 将 list x 接合于 pos 之前
template <class T>
void list<T>::splice(const_iterator pos, list& x)
{
  YASTL_DEBUG(this != &x);
  if (!x.empty())
  {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - x.size_, "list<T>'s size too big");

    auto f = x.node_->next;
    auto l = x.node_->prev;

    x.unlink_nodes(f, l);
    link_nodes(pos.node_, f, l);

    size_ += x.size_;
    x.size_ = 0;
  }
}

// 将 it 所指的节点接合于 pos 之前
template <class T>
void list<T>::splice(const_iterator pos, list& x, const_iterator it)
{
  if (pos.node_ != it.node_ && pos.node_ != it.node_->next)
  {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");

    auto f = it.node_;

    x.unlink_nodes(f, f);
    link_nodes(pos.node_, f, f);

    ++size_;
    --x.size_;
  }
}

// 将 list x 的 [first, last) 内的节点接合于 pos 之前
template <class T>
void list<T>::splice(const_iterator pos, list& x, const_iterator first, const_iterator last)
{
  if (first != last && this != &x)
  {
    size_type n = yastl::distance(first, last);
    THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
    auto f = first.node_;
    auto l = last.node_->prev;

    x.unlink_nodes(f, l);
    link_nodes(pos.node_, f, l);

    size_ += n;
    x.size_ -= n;
  }
}

// 将另一元操作 pred 为 true 的所有元素移除
template <class T>
template <class UnaryPredicate>
void list<T>::remove_if(UnaryPredicate pred) {
  auto f = begin();
  auto l = end();
  for (auto next = f; f != l; f = next) {
    ++next;
    if (pred(*f)) { // pred本质是个函数，接受一个参数称之为一元谓词
      erase(f);
    }
  }
}

// 移除 list 中满足 pred 为 true 重复元素
template <class T>
template <class BinaryPredicate>
void list<T>::unique(BinaryPredicate pred) {
  auto i = begin();
  auto e = end();
  auto j = i;
  ++j;
  while (j != e) { // 遍历全部
    if (pred(*i, *j)) { // 满足二元谓词
      erase(j);
    } else {
      i = j;
    }
    j = i;
    ++j;
  }
}

// 与另一个 list 合并，按照 comp 为 true 的顺序
template <class T>
template <class Compare>
void list<T>::merge(list& x, Compare comp) {
  if (this != &x) {
    THROW_LENGTH_ERROR_IF(size_ > max_size() - x.size_, "list<T>'s size too big");

    auto f1 = begin();
    auto l1 = end();
    auto f2 = x.begin();
    auto l2 = x.end();

    while (f1 != l1 && f2 != l2) {
      if (comp(*f2, *f1)) {  // x的节点比当前值小
        // 使 comp 为 true 的一段区间
        auto next = f2;
        ++next;
        for (; next != l2 && comp(*next, *f1); ++next) // 遍历直到一个比f1值大的
          ;
        auto f = f2.node_;
        auto l = next.node_->prev;
        f2 = next;

        // link node
        x.unlink_nodes(f, l);
        link_nodes(f1.node_, f, l); // 把这一段都插在f1前面
        ++f1;
      } else { // x的节点比当前值大
        ++f1;
      }
    }
    // 连接剩余部分
    if (f2 != l2) {
      auto f = f2.node_;
      auto l = l2.node_->prev;
      x.unlink_nodes(f, l);
      link_nodes(l1.node_, f, l);
    }

    size_ += x.size_;
    x.size_ = 0;
  }
}

// 将 list 反转
template <class T>
void list<T>::reverse()
{
  if (size_ <= 1)
  {
    return;
  }
  auto i = begin();
  auto e = end();
  while (i.node_ != e.node_)
  {
    yastl::swap(i.node_->prev, i.node_->next);
    i.node_ = i.node_->prev;
  }
  yastl::swap(e.node_->prev, e.node_->next);
}

/*****************************************************************************************/
// helper function

// 创建结点
template <class T>
template <class ...Args>
typename list<T>::node_ptr 
list<T>::create_node(Args&& ...args) {
  node_ptr p = node_allocator::allocate(1);
  try {
    data_allocator::construct(yastl::address_of(p->value), yastl::forward<Args>(args)...);
    p->prev = nullptr;
    p->next = nullptr;
  }
  catch (...)
  {
    node_allocator::deallocate(p);
    throw;
  }
  return p;
}

// 销毁结点，析构并释放内存
template <class T>
void list<T>::destroy_node(node_ptr p)
{
  data_allocator::destroy(yastl::address_of(p->value));
  node_allocator::deallocate(p);
}

// 用 n 个元素初始化容器
template <class T>
void list<T>::fill_init(size_type n, const value_type& value) {
  node_ = base_allocator::allocate(1);
  node_->unlink();
  size_ = n;
  try {
    for (; n > 0; --n) {
      auto node = create_node(value);
      link_nodes_at_back(node->as_base(), node->as_base());
    }
  }
  catch (...)
  {
    clear();
    base_allocator::deallocate(node_);
    node_ = nullptr;
    throw;
  }
}

// 以 [first, last) 初始化容器
template <class T>
template <class Iter>
void list<T>::copy_init(Iter first, Iter last)
{
  node_ = base_allocator::allocate(1);
  node_->unlink();
  size_type n = yastl::distance(first, last);
  size_ = n;
  try
  {
    for (; n > 0; --n, ++first)
    {
      auto node = create_node(*first);
      link_nodes_at_back(node->as_base(), node->as_base());
    }
  }
  catch (...)
  {
    clear();
    base_allocator::deallocate(node_);
    node_ = nullptr;
    throw;
  }
}

// 在 pos 处连接一个节点
template <class T>
typename list<T>::iterator 
list<T>::link_iter_node(const_iterator pos, base_ptr link_node) {
  if (pos == node_->next) { // 在开头插
    link_nodes_at_front(link_node, link_node);
  }
  else if (pos == node_) { // 在尾部插
    link_nodes_at_back(link_node, link_node);
  } else { // 在pos插入
    link_nodes(pos.node_, link_node, link_node);
  }
  return iterator(link_node);
}

// 在 pos 处连接 [first, last] 的结点
template <class T>
void list<T>::link_nodes(base_ptr pos, base_ptr first, base_ptr last) {
  pos->prev->next = first;
  first->prev = pos->prev;
  pos->prev = last;
  last->next = pos;
}

// 在头部连接 [first, last] 结点
template <class T>
void list<T>::link_nodes_at_front(base_ptr first, base_ptr last) {
  first->prev = node_;
  last->next = node_->next;
  last->next->prev = last;
  node_->next = first; // 设置头节点为first
}

// 在尾部连接 [first, last] 结点
template <class T>
void list<T>::link_nodes_at_back(base_ptr first, base_ptr last)
{
  last->next = node_;
  first->prev = node_->prev;
  first->prev->next = first;
  node_->prev = last;
}

// 容器与 [first, last] 结点断开连接,把这段扣除
template <class T>
void list<T>::unlink_nodes(base_ptr first, base_ptr last) {
  first->prev->next = last->next;
  last->next->prev = first->prev;
}

// 用 n 个元素为容器赋值
template <class T>
void list<T>::fill_assign(size_type n, const value_type& value)
{
  auto i = begin();
  auto e = end();
  for (; n > 0 && i != e; --n, ++i)
  {
    *i = value;
  }
  if (n > 0)
  {
    insert(e, n, value);
  }
  else
  {
    erase(i, e);
  }
}

// 复制[f2, l2)为容器赋值
template <class T>
template <class Iter>
void list<T>::copy_assign(Iter f2, Iter l2)
{
  auto f1 = begin();
  auto l1 = end();
  for (; f1 != l1 && f2 != l2; ++f1, ++f2)
  {
    *f1 = *f2;
  }
  if (f2 == l2)
  {
    erase(f1, l1);
  }
  else
  {
    insert(l1, f2, l2);
  }
}

// 在 pos 处插入 n 个元素
template <class T>
typename list<T>::iterator 
list<T>::fill_insert(const_iterator pos, size_type n, const value_type& value)
{
  iterator r(pos.node_);
  if (n != 0)
  {
    const auto add_size = n;
    auto node = create_node(value);
    node->prev = nullptr;
    r = iterator(node);
    iterator end = r;
    try
    {
      // 前面已经创建了一个节点，还需 n - 1 个
      for (--n; n > 0; --n, ++end)
      {
        auto next = create_node(value);
        end.node_->next = next->as_base();  // link node
        next->prev = end.node_;
      }
      size_ += add_size;
    }
    catch (...)
    {
      auto enode = end.node_;
      while (true)
      {
        auto prev = enode->prev;
        destroy_node(enode->as_node());
        if (prev == nullptr)
          break;
        enode = prev;
      }
      throw;
    }
    link_nodes(pos.node_, r.node_, end.node_);
  }
  return r;
}

// 在 pos 处插入 [first, last) 的元素
template <class T>
template <class Iter>
typename list<T>::iterator 
list<T>::copy_insert(const_iterator pos, size_type n, Iter first)
{
  iterator r(pos.node_);
  if (n != 0)
  {
    const auto add_size = n;
    auto node = create_node(*first);
    node->prev = nullptr;
    r = iterator(node);
    iterator end = r;
    try
    {
      for (--n, ++first; n > 0; --n, ++first, ++end)
      {
        auto next = create_node(*first);
        end.node_->next = next->as_base();  // link node
        next->prev = end.node_;
      }
      size_ += add_size;
    }
    catch (...)
    {
      auto enode = end.node_;
      while (true)
      {
        auto prev = enode->prev;
        destroy_node(enode->as_node());
        if (prev == nullptr)
          break;
        enode = prev;
      }
      throw;
    }
    link_nodes(pos.node_, r.node_, end.node_);
  }
  return r;
}

// 对 list 进行归并排序，返回一个迭代器指向区间最小元素的位置
template <class T>
template <class Compared>
typename list<T>::iterator 
list<T>::list_sort(iterator f1, iterator l2, size_type n, Compared comp) {
  if (n < 2) { // 一个不用排序
    return f1;
  }

  if (n == 2) {
    if (comp(*--l2, *f1)) {
      auto ln = l2.node_;
      unlink_nodes(ln, ln);
      link_nodes(f1.node_, ln, ln);
      return l2;
    }
    return f1;
  }

  auto n2 = n / 2;
  auto l1 = f1;
  yastl::advance(l1, n2);
  auto result = f1 = list_sort(f1, l1, n2, comp);  // 前半段的最小位置
  auto f2 = l1 = list_sort(l1, l2, n - n2, comp);  // 后半段的最小位置

  // 把较小的一段区间移到前面
  if (comp(*f2, *f1)) {
    auto m = f2;
    ++m;
    for (; m != l2 && comp(*m, *f1); ++m)
      ;
    auto f = f2.node_;
    auto l = m.node_->prev;
    result = f2;
    l1 = f2 = m;
    unlink_nodes(f, l);
    m = f1;
    ++m;
    link_nodes(f1.node_, f, l);
    f1 = m;
  } else {
    ++f1;
  }

  // 合并两段有序区间
  while (f1 != l1 && f2 != l2) {
    if (comp(*f2, *f1)) {
      auto m = f2;
      ++m;
      for (; m != l2 && comp(*m, *f1); ++m)
        ;
      auto f = f2.node_;
      auto l = m.node_->prev;
      if (l1 == f2) {
        l1 = m;
      }
      f2 = m;
      unlink_nodes(f, l);
      m = f1;
      ++m;
      link_nodes(f1.node_, f, l);
      f1 = m;
    } else {
      ++f1;
    }
  }
  return result;
}

// 重载比较操作符
template <class T>
bool operator==(const list<T>& lhs, const list<T>& rhs)
{
  auto f1 = lhs.cbegin();
  auto f2 = rhs.cbegin();
  auto l1 = lhs.cend();
  auto l2 = rhs.cend();
  for (; f1 != l1 && f2 != l2 && *f1 == *f2; ++f1, ++f2)
    ;
  return f1 == l1 && f2 == l2;
}

template <class T>
bool operator<(const list<T>& lhs, const list<T>& rhs)
{
  return yastl::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <class T>
bool operator!=(const list<T>& lhs, const list<T>& rhs)
{
  return !(lhs == rhs);
}

template <class T>
bool operator>(const list<T>& lhs, const list<T>& rhs)
{
  return rhs < lhs;
}

template <class T>
bool operator<=(const list<T>& lhs, const list<T>& rhs)
{
  return !(rhs < lhs);
}

template <class T>
bool operator>=(const list<T>& lhs, const list<T>& rhs)
{
  return !(lhs < rhs);
}

// 重载 yastl 的 swap
template <class T>
void swap(list<T>& lhs, list<T>& rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace yastl
#endif // _INCLUDE_LIST_H_

