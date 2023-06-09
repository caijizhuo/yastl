﻿#ifndef _INCLUDE_UNINITIALIZED_H_
#define _INCLUDE_UNINITIALIZED_H_

// 这个头文件用于对未初始化空间构造元素

#include "algobase.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"
#include "util.h"

namespace yastl {

/*****************************************************************************************/
// uninitialized_copy
// 把 [first, last) 上的内容复制到以 result 为起始处的空间，返回复制结束的位置
/*****************************************************************************************/
// 会调用拷贝构造,构造函数不重要版本
template <class InputIter, class ForwardIter>
ForwardIter unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::true_type) {
  return yastl::copy(first, last, result); // 只复制这一段的内存 为浅拷贝
}

// 构造函数重要版本
template <class InputIter, class ForwardIter>
ForwardIter unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result, std::false_type) {
  auto cur = result;
  try {
    for (; first != last; ++first, ++cur) {
      yastl::construct(&*cur, *first);  // 调用构造函数 可能为深拷贝
    }
  } catch (...) { // 有异常就全部释放
    for (; result != cur; --cur) {
      yastl::destroy(&*cur);
    }
  }
  return cur; // 返回尾部
}

// 泛型总入口,要求此函数要么构造所有，要么一个都不构造，
// 把 [first, last) 上的内容复制到以 result 为起始处的空间，返回复制结束的位置, 拷贝构造
template <class InputIter, class ForwardIter>
ForwardIter uninitialized_copy(InputIter first, InputIter last, ForwardIter result) {
  return yastl::unchecked_uninit_copy(first, last, result, 
                                     std::is_trivially_copy_assignable<   // 判断是否是无用的拷贝构造函数 plain old data
                                     typename iterator_traits<ForwardIter>::value_type>{});  
}

/*****************************************************************************************/
// uninitialized_copy_n
// 把 [first, first + n) 上的内容复制到以 result 为起始处的空间，返回复制结束的位置
/*****************************************************************************************/
// 构造函数不重要版本
template <class InputIter, class Size, class ForwardIter>
ForwardIter unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::true_type) {
  return yastl::copy_n(first, n, result).second;
}
// 构造函数重要版本
template <class InputIter, class Size, class ForwardIter>
ForwardIter unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result, std::false_type) {
  auto cur = result;
  try {
    for (; n > 0; --n, ++cur, ++first) {
      yastl::construct(&*cur, *first);
    }
  } catch (...) {
    for (; result != cur; --cur) {
      yastl::destroy(&*cur);
    } 
  }
  return cur;
}
// 把 [first, first + n) 上的内容复制到以 result 为起始处的空间，返回复制结束的位置
template <class InputIter, class Size, class ForwardIter>
ForwardIter uninitialized_copy_n(InputIter first, Size n, ForwardIter result) {
  return yastl::unchecked_uninit_copy_n(first, n, result,
                                        std::is_trivially_copy_assignable<
                                        typename iterator_traits<InputIter>::
                                        value_type>{});
}

/*****************************************************************************************/
// uninitialized_fill
// 在 [first, last) 区间内填充元素值
/*****************************************************************************************/
// 构造函数不重要
template <class ForwardIter, class T>
void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value, std::true_type) {
  yastl::fill(first, last, value);
}
// 构造函数重要
template <class ForwardIter, class T>
void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value, std::false_type) {
  auto cur = first;
  try {
    for (; cur != last; ++cur) {
      yastl::construct(&*cur, value);
    }
  } catch (...) {
    for (;first != cur; ++first) {
      yastl::destroy(&*first);
    }
  }
}

// 在 [first, last) 区间内填充元素值
template <class ForwardIter, class T>
void uninitialized_fill(ForwardIter first, ForwardIter last, const T& value) {
  yastl::unchecked_uninit_fill(first, last, value, 
                               std::is_trivially_copy_assignable<
                               typename iterator_traits<ForwardIter>::value_type>{});
}

/*****************************************************************************************/
// uninitialized_fill_n
// 从 first 位置开始，填充 n 个元素值，返回填充结束的位置
/*****************************************************************************************/
// 构造函数重要版本
template <class ForwardIter, class Size, class T>
ForwardIter unchecked_uninit_fill_n(ForwardIter first, Size n, const T& value, std::true_type) {
  return yastl::fill_n(first, n, value);
}
// 构造函数不重要版本
template <class ForwardIter, class Size, class T>
ForwardIter unchecked_uninit_fill_n(ForwardIter first, Size n, const T& value, std::false_type) {
  auto cur = first;
  try {
    for (; n > 0; --n, ++cur) {
      yastl::construct(&*cur, value);
    }
  } catch (...) {
    for (; first != cur; ++first) {
      yastl::destroy(&*first);
    }
  }
  return cur;
}
// 从first填充n个value
template <class ForwardIter, class Size, class T>
ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T& value) {
  return yastl::unchecked_uninit_fill_n(first, n, value, 
                                        std::is_trivially_copy_assignable<
                                        typename iterator_traits<ForwardIter>::
                                        value_type>{});
}

/*****************************************************************************************/
// uninitialized_move
// 把[first, last)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
/*****************************************************************************************/
// 不需要调用移动构造函数版本
template <class InputIter, class ForwardIter>
ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::true_type) {
  return yastl::move(first, last, result); // 可以直接移动
}
// 需要调用移动构造函数版本
template <class InputIter, class ForwardIter>
ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::false_type) {
  ForwardIter cur = result;
  try {
    for (; first != last; ++first, ++cur) {
      yastl::construct(&*cur, yastl::move(*first)); // 批量调用移动构造函数
    }
  } catch (...) {
    yastl::destroy(result, cur);
  }
  return cur;
}

// 把[first, last)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
template <class InputIter, class ForwardIter>
ForwardIter uninitialized_move(InputIter first, InputIter last, ForwardIter result) {
  return yastl::unchecked_uninit_move(first, last, result,
                                      std::is_trivially_move_assignable<  // 平凡可移动
                                      typename iterator_traits<InputIter>::
                                      value_type>{});
}

/*****************************************************************************************/
// uninitialized_move_n
// 把[first, first + n)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
/*****************************************************************************************/
// 不需要调用移动构造函数版本
template <class InputIter, class Size, class ForwardIter>
ForwardIter unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::true_type) {
  return yastl::move(first, first + n, result);
}
// 需要调用移动构造函数版本
template <class InputIter, class Size, class ForwardIter>
ForwardIter unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::false_type) {
  auto cur = result;
  try {
    for (; n > 0; --n, ++first, ++cur) {
      yastl::construct(&*cur, yastl::move(*first)); // 调用移动构造函数
    }
  } catch (...) {
    for (; result != cur; ++result) {
      yastl::destroy(&*result);
    }
    throw;
  }
  return cur;
}
// 把[first, first + n)上的内容移动到以 result 为起始处的空间，返回移动结束的位置
template <class InputIter, class Size, class ForwardIter>
ForwardIter uninitialized_move_n(InputIter first, Size n, ForwardIter result) {
  return yastl::unchecked_uninit_move_n(first, n, result,
                                        std::is_trivially_move_assignable<
                                        typename iterator_traits<InputIter>::value_type>{});
}
} // namespace yastl
#endif // _INCLUDE_UNINITIALIZED_H_

