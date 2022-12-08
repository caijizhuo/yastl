#ifndef _INCLUDE_CONSTRUCT_H_
#define _INCLUDE_CONSTRUCT_H_

// 这个头文件包含两个函数 construct，destroy
// construct : 负责对象的构造
// destroy   : 负责对象的析构

#include <new>

#include "type_traits.h"
#include "iterator.h"

namespace yastl
{

// construct 构造对象

template <class Ty>
void construct(Ty* ptr)
{
  ::new ((void*)ptr) Ty(); // placement new 在已经分配好空间的内存中创建对象Ty，ptr为内存地址
}

template <class Ty1, class Ty2>
void construct(Ty1* ptr, const Ty2& value)
{
  ::new ((void*)ptr) Ty1(value);
}

template <class Ty, class... Args>
void construct(Ty* ptr, Args&&... args)
{
  ::new ((void*)ptr) Ty(yastl::forward<Args>(args)...);
}

// destroy 将对象析构

template <class Ty>
void destroy_one(Ty*, std::true_type) {} // 一个迭代器析构函数不重要 true type

template <class Ty>
void destroy_one(Ty* pointer, std::false_type) // 析构函数重要 要调用 false type
{
  if (pointer != nullptr)
  {
    pointer->~Ty();
  }
}

template <class ForwardIter>
void destroy_cat(ForwardIter , ForwardIter , std::true_type) {} // 一堆迭代器 析构函数不重要 true type

template <class ForwardIter>
void destroy_cat(ForwardIter first, ForwardIter last, std::false_type) { // 一堆迭代器 析构函数重要 要调用 false type
  for (; first != last; ++first) {
    destroy(&*first);
  }
}

// 总接口destroy 分为一个入参和一堆迭代器，让模板自动匹配
template <class Ty>
void destroy(Ty* pointer) {
  destroy_one(pointer, std::is_trivially_destructible<Ty>{}); // 如果定义了非默认的析构函数，则返回true
}

template <class ForwardIter>
void destroy(ForwardIter first, ForwardIter last) {
  destroy_cat(first, last, std::is_trivially_destructible<typename iterator_traits<ForwardIter>::value_type>{});
}

} // namespace yastl
#endif // _INCLUDE_CONSTRUCT_H_

