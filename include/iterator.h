﻿#ifndef _INCLUDE_ITERATOR_H_
#define _INCLUDE_ITERATOR_H_

// 这个头文件用于迭代器设计，包含了一些模板结构体与全局函数，

#include <cstddef>

#include "type_traits.h"

namespace yastl {

// 五种迭代器类型
// 输入迭代器指向的位置只能被顺序读取
struct input_iterator_tag {};
// 输出迭代器指向的位置只能被顺序写入
struct output_iterator_tag {};
// 前向迭代器只能自增不能自减
struct forward_iterator_tag : public input_iterator_tag {};
// 双向迭代器既能自增，又能自减
struct bidirectional_iterator_tag : public forward_iterator_tag {};
// 随机访问迭代器，是一种特殊的双向迭代器，除了自增自减1之外，还能自增自减n
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

// iterator 模板
template <class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
struct iterator {
  typedef Category iterator_category;
  typedef T value_type;
  typedef Pointer pointer;
  typedef Reference reference;
  typedef Distance difference_type;
};

// iterator traits
// 此类模板用于判断iter是否含有iterator_category，没有则为原生类型
// 有个萃取前提，只有对有 iterator_category这个属性，并且可以转换为input_iterator_tag和output_iterator_tag的iterator萃取。
template <class T>
struct has_iterator_cat {
 private:
  struct two { char a; char b; };

  template <class U>
  static two test(...);

  // 运用了SFINAE技巧，如果有iterator_category属性，会匹配到这里
  template <class U>
  static char test(typename U::iterator_category* = 0);
 public:
  // 返回值是char则是跑的下面的，否则是原生类型
  static const bool value = sizeof(test<T>(0)) == sizeof(char);
};

// 原生类型走这个实现
template <class Iterator, bool>
struct iterator_traits_impl {};

// 如果第二个泛型为true，则编译此结构体，说明是迭代器，走这个类型
template <class Iterator>
struct iterator_traits_impl<Iterator, true> {
  typedef typename Iterator::iterator_category iterator_category;
  typedef typename Iterator::value_type value_type;
  typedef typename Iterator::pointer pointer;
  typedef typename Iterator::reference reference;
  typedef typename Iterator::difference_type difference_type;
};
// 给基础类型用的
template <class Iterator, bool>
struct iterator_traits_helper {};
// 给迭代器类型用的
template <class Iterator>
struct iterator_traits_helper<Iterator, true> : 
    public iterator_traits_impl<Iterator, std::is_convertible<typename Iterator::iterator_category, input_iterator_tag>::value ||
                                std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::value>
                                // 看 Iterator::iterator_categor是否可以转换为output_iterator_tag和input_iterator_tag
{};

// 萃取迭代器的特性
// 萃取迭代器的目的就是输入一个迭代器，把元素的type给萃取出来，继承自helper,helper再继承impl
/*
- 如果是原生指针，会匹配到下面两个，typedef的定义为random
- 如果是类类型，则会匹配到主模板，并根据has_iterator_cat类模板(根据value可判断类T中是否具有iterator_category属性)，
如果为true，进而继承于iterator_traits_impl，最后将根据is_convertible类模板判断可否将类T转换为input和output指针决定是否进行
typedef一系列操作
*/
template <class Iterator>
struct iterator_traits : public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

// 针对原生指针的偏特化版本,类型都是random_access_iterator_tag
template <class T>
struct iterator_traits<T*> {
  typedef random_access_iterator_tag iterator_category;
  typedef T value_type;
  typedef T* pointer;
  typedef T& reference;
  typedef ptrdiff_t difference_type;
};
// 原生const指针的偏特化版本，如果不加这个函数那通过原生指针获得的value_type就是const T了
template <class T>
struct iterator_traits<const T*> {
  typedef random_access_iterator_tag iterator_category;
  typedef T value_type;
  typedef const T* pointer;
  typedef const T& reference;
  typedef ptrdiff_t difference_type;
};

template <class T, class U, bool = has_iterator_cat<iterator_traits<T>>::value>
struct has_iterator_cat_of : 
  public m_bool_constant<std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value>
{};

// 萃取某种迭代器，这5个struct会在algo.h的算法中用来判断迭代器种类，来选择不同的算法
template <class T, class U>
struct has_iterator_cat_of<T, U, false> : public m_false_type {};

template <class Iter>
struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_tag> {};

template <class Iter>
struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};

template <class Iter>
struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

template <class Iter>
struct is_bidirectional_iterator : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

template <class Iter>
struct is_random_access_iterator : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

template <class Iterator>
struct is_iterator : // 条件是迭代器输入输出的tag至少有一个为真
  public m_bool_constant<is_input_iterator<Iterator>::value || is_output_iterator<Iterator>::value>
{};

// 萃取某个迭代器的 category
template <class Iterator>
typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator&) {
  typedef typename iterator_traits<Iterator>::iterator_category Category;
  return Category(); // 返回临时对象
}

// 萃取某个迭代器的 distance_type
template <class Iterator>
typename iterator_traits<Iterator>::difference_type* distance_type(const Iterator&) {
  return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}

// 萃取某个迭代器的 value_type
template <class Iterator>
typename iterator_traits<Iterator>::value_type* value_type(const Iterator&) {
  return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

// 以下函数用于计算迭代器间的距离

// distance 的 input_iterator_tag 的版本
template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type 
distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag) {
  typename iterator_traits<InputIterator>::difference_type n = 0;
  while (first != last) {
    ++first;
    ++n;
  }
  return n;
}

// distance 的 random_access_iterator_tag 的版本
template <class RandomIter>
typename iterator_traits<RandomIter>::difference_type
distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag) {
  return last - first;
}

// 返回first到last的距离
template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last) {
  return distance_dispatch(first, last, iterator_category(first));
}

// 以下函数用于让迭代器前进 n 个距离

// advance 的 input_iterator_tag 的版本
template <class InputIterator, class Distance>
void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag) {
  while (n--) {
    ++i;
  }
}

// advance 的 bidirectional_iterator_tag 的版本
template <class BidirectionalIterator, class Distance>
void advance_dispatch(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag) {
  if (n >= 0) {
    while (n--) {
      ++i;
    }
  } else {
    while (n++) {
      --i;
    }
  }
}

// advance 的 random_access_iterator_tag 的版本
template <class RandomIter, class Distance>
void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag) {
  i += n;
}

// 总入口，让它来决定编译哪种advance_dispatch，让迭代器i前进n次
template <class InputIterator, class Distance>
void advance(InputIterator& i, Distance n) {
  advance_dispatch(i, n, iterator_category(i));
}

/*****************************************************************************************/

// 模板类 : reverse_iterator
// 代表反向迭代器，使前进为后退，后退为前进
template <class Iterator>
class reverse_iterator {
private:
  Iterator current;  // 记录对应的正向迭代器

public:
  // 反向迭代器的五种相应型别
  typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
  typedef typename iterator_traits<Iterator>::value_type value_type;
  typedef typename iterator_traits<Iterator>::difference_type difference_type;
  typedef typename iterator_traits<Iterator>::pointer pointer;
  typedef typename iterator_traits<Iterator>::reference reference;

  typedef Iterator iterator_type;
  typedef reverse_iterator<Iterator> self;

public:
  // 构造函数
  reverse_iterator() {}
  explicit reverse_iterator(iterator_type i) : current(i) {}
  reverse_iterator(const self& rhs) : current(rhs.current) {}

public:
  // 取出对应的正向迭代器
  iterator_type base() const {
    return current;
  }

  // 重载操作符
  reference operator*() const { // 实际对应正向迭代器的前一个位置
    auto tmp = current;
    return *--tmp;
  }
  pointer operator->() const {
    return &(operator*());
  }

  // 前进(++)变为后退(--)
  self& operator++() {
    --current;
    return *this;
  }
  self operator++(int) {
    self tmp = *this;
    --current;
    return tmp;
  }
  // 后退(--)变为前进(++)
  self& operator--() {
    ++current;
    return *this;
  }
  self operator--(int) {
    self tmp = *this;
    ++current;
    return tmp;
  }

  self& operator+=(difference_type n) {
    current -= n;
    return *this;
  }
  self operator+(difference_type n) const {
    return self(current - n);
  }
  self& operator-=(difference_type n) {
    current += n;
    return *this;
  }
  self operator-(difference_type n) const {
    return self(current + n);
  }

  reference operator[](difference_type n) const {
    return *(*this + n);
  }
};

// 重载 operator-
template <class Iterator>
typename reverse_iterator<Iterator>::difference_type operator-(const reverse_iterator<Iterator>& lhs,
                                                               const reverse_iterator<Iterator>& rhs) {
  return rhs.base() - lhs.base();
}

// 重载比较操作符  指针的相等
template <class Iterator>
bool operator==(const reverse_iterator<Iterator>& lhs,
                const reverse_iterator<Iterator>& rhs) {
  return lhs.base() == rhs.base();
}

template <class Iterator>
bool operator<(const reverse_iterator<Iterator>& lhs,
               const reverse_iterator<Iterator>& rhs) {
  return rhs.base() < lhs.base();
}

template <class Iterator>
bool operator!=(const reverse_iterator<Iterator>& lhs,
                const reverse_iterator<Iterator>& rhs) {
  return !(lhs == rhs);
}

// 大于变小于
template <class Iterator>
bool operator>(const reverse_iterator<Iterator>& lhs,
               const reverse_iterator<Iterator>& rhs) {
  return rhs < lhs;
}

// 小于等于为不大于 为了使用刚重载的<操作符
template <class Iterator>
bool operator<=(const reverse_iterator<Iterator>& lhs,
                const reverse_iterator<Iterator>& rhs) {
  return !(rhs < lhs);
}

// 大于等于为不小于 为了只用重载的<操作符
template <class Iterator>
bool operator>=(const reverse_iterator<Iterator>& lhs,
                const reverse_iterator<Iterator>& rhs) {
  return !(lhs < rhs);
}
} // namespace yastl

#endif // _INCLUDE_ITERATOR_H_

