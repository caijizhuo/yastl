#ifndef _INCLUDE_ALLOCATOR_H_
#define _INCLUDE_ALLOCATOR_H_

// 这个头文件包含一个模板类 allocator，用于管理内存的分配、释放，对象的构造、析构

#include "construct.h"
#include "util.h"

namespace yastl {
// 模板类：allocator
// 模板函数代表数据类型
template <class T>
class allocator {
public:
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

public:
  static T* allocate(); // 分配内存
  static T* allocate(size_type n);

  static void deallocate(T* ptr); // 释放内存
  static void deallocate(T* ptr, size_type n);

  static void construct(T* ptr); // 构建对象
  // copy construct
  static void construct(T* ptr, const T& value);
  // move construct
  static void construct(T* ptr, T&& value);

  // 泛型工厂函数，作为参数的路由，通过forward按照参数实际类型去匹配对应重载函数，实现完美转发
  // 这个函数可以实现创建所有类型的对象，如果参数实际值是右值，创建会自动调用移动构造，如果是左值会匹配拷贝构造
  template <class... Args>
  static void construct(T* ptr, Args&& ...args);

  static void destroy(T* ptr); // 析构对象
  static void destroy(T* first, T* last);
};

template <class T>
T* allocator<T>::allocate() {
  return static_cast<T*>(::operator new(sizeof(T)));
}

template <class T>
T* allocator<T>::allocate(size_type n) {
  if (n == 0) {
    return nullptr;
  }
  return static_cast<T*>(::operator new(n * sizeof(T)));
}

template <class T>
void allocator<T>::deallocate(T* ptr) {
  if (ptr == nullptr) {
    return;
  }
  ::operator delete(ptr);
}

template <class T>
void allocator<T>::deallocate(T* ptr, size_type /*size*/) {
  if (ptr == nullptr) {
    return;
  }
  ::operator delete(ptr);
}

// 根据指针构造
template <class T>
void allocator<T>::construct(T* ptr) {
  yastl::construct(ptr);
}

// 拷贝构造分配
template <class T>
void allocator<T>::construct(T* ptr, const T& value) {
  yastl::construct(ptr, value);
}

// 移动构造分配
template <class T>
void allocator<T>::construct(T* ptr, T&& value) {
  yastl::construct(ptr, yastl::move(value));
}

// 完美转发构造，且多个参数
template <class T>
template <class ...Args>
void allocator<T>::construct(T* ptr, Args&& ...args) {
  yastl::construct(ptr, yastl::forward<Args>(args)...);
}

// 析构一个迭代器指向的内容
template <class T>
void allocator<T>::destroy(T* ptr) {
  yastl::destroy(ptr);
}

// 析构一堆迭代器指向的内容
template <class T>
void allocator<T>::destroy(T* first, T* last) {
  yastl::destroy(first, last);
}

} // namespace yastl
#endif // _INCLUDE_ALLOCATOR_H_

