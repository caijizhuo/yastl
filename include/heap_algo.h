#ifndef _INCLUDE_HEAP_ALGO_H_
#define _INCLUDE_HEAP_ALGO_H_

// 这个头文件包含 heap 的四个算法 : push_heap, pop_heap, sort_heap, make_heap
// 本文件是以0作为根节点，左孩子和右孩子为2N + 1 和 2N + 2， 父亲节点为 (N - 1) / 2

#include "iterator.h"

namespace yastl {

/*****************************************************************************************/
// push_heap
// 该函数接受两个迭代器，表示一个 heap 容器的首尾，并且新元素已经插入到底部容器的最尾端，调整 heap
/*****************************************************************************************/
template <class RandomIter, class Distance, class T>
void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value) {
  auto parent = (holeIndex - 1) / 2;
  while (holeIndex > topIndex && *(first + parent) < value) { // 如果没到根节点并且parent节点比value小，需要进行上滤（大顶堆）
    // 使用 operator<，所以 heap 为 max-heap
    *(first + holeIndex) = *(first + parent); // 先把hole的值填上父节点的值
    holeIndex = parent; // 继续更新hole的位置
    parent = (holeIndex - 1) / 2;
  }
  *(first + holeIndex) = value;
}

// 在last是value的情况下进行上滤
template <class RandomIter, class Distance>
void push_heap_d(RandomIter first, RandomIter last, Distance*) {
  yastl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1));
}

// 在last是value的情况下进行上滤
template <class RandomIter>
void push_heap(RandomIter first, RandomIter last) { // 新元素应该已置于底部容器的最尾端
  yastl::push_heap_d(first, last, distance_type(first));
}

// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class Distance, class T, class Compared>
void push_heap_aux(RandomIter first, Distance holeIndex, Distance topIndex, T value, Compared comp) {
  auto parent = (holeIndex - 1) / 2;
  while (holeIndex > topIndex && comp(*(first + parent), value)) {
    *(first + holeIndex) = *(first + parent); // 上滤
    holeIndex = parent;
    parent = (holeIndex - 1) / 2;
  }
  *(first + holeIndex) = value;
}

template <class RandomIter, class Compared, class Distance>
void push_heap_d(RandomIter first, RandomIter last, Distance*, Compared comp) {
  yastl::push_heap_aux(first, (last - first) - 1, static_cast<Distance>(0), *(last - 1), comp);
}

template <class RandomIter, class Compared>
void push_heap(RandomIter first, RandomIter last, Compared comp) {
  yastl::push_heap_d(first, last, distance_type(first), comp);
}

/*****************************************************************************************/
// pop_heap
// 该函数接受两个迭代器，表示 heap 容器的首尾，将 heap 的根节点取出放到容器尾部，调整 heap
/*****************************************************************************************/
template <class RandomIter, class T, class Distance>
void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value) {
  // 先进行下溯(percolate down)过程
  auto topIndex = holeIndex;
  auto rchild = 2 * holeIndex + 2;
  while (rchild < len) {
    if (*(first + rchild) < *(first + rchild - 1)) { // 右孩子小于左孩子，将左孩子标记为更大的
      --rchild; // 此处为左孩子的下标
    }
    *(first + holeIndex) = *(first + rchild); // 把较大的填过来，继续向下
    holeIndex = rchild;
    rchild = 2 * (rchild + 1);
  }
  if (rchild == len) {  // 如果没有右子节点
    *(first + holeIndex) = *(first + (rchild - 1)); // 就用左子节点填
    holeIndex = rchild - 1;
  }
  // 再执行一次上溯(percolate up)过程
  yastl::push_heap_aux(first, holeIndex, topIndex, value);
}

template <class RandomIter, class T, class Distance>
void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance*) {
  // 先将首值调至尾节点，然后调整[first, last - 1)使之重新成为一个 max-heap
  *result = *first;
  yastl::adjust_heap(first, static_cast<Distance>(0), last - first, value);
}

template <class RandomIter>
void pop_heap(RandomIter first, RandomIter last) {
  yastl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first));
}

// 重载版本使用函数对象 comp 代替比较操作 关键字heapify
// 在holeindex位置插入value，但是会先调整holeindex的位置
// 插入value的时候需要上滤
template <class RandomIter, class T, class Distance, class Compared>
void adjust_heap(RandomIter first, Distance holeIndex, Distance len, T value, Compared comp) {
  // 先进行下溯(percolate down)过程
  auto topIndex = holeIndex;
  auto rchild = 2 * holeIndex + 2;
  while (rchild < len) {
    if (comp(*(first + rchild), *(first + rchild - 1))) { // 比较左右孩子获得更comp的那个下标
      --rchild;
    }
    *(first + holeIndex) = *(first + rchild);
    holeIndex = rchild; // 下移一层
    rchild = 2 * (rchild + 1); // 获得其右孩子
  }
  if (rchild == len) { // 到头了,并且只有左孩子
    *(first + holeIndex) = *(first + (rchild - 1)); // 设置为左孩子
    holeIndex = rchild - 1;
  }
  // 再执行一次上溯(percolate up)过程
  yastl::push_heap_aux(first, holeIndex, topIndex, value, comp);
}

template <class RandomIter, class T, class Distance, class Compared>
void pop_heap_aux(RandomIter first, RandomIter last, RandomIter result, T value, Distance*, Compared comp) {
  *result = *first;  // 先将尾指设置成首值，即尾指为欲求结果
  yastl::adjust_heap(first, static_cast<Distance>(0), last - first, value, comp);
}

template <class RandomIter, class Compared>
void pop_heap(RandomIter first, RandomIter last, Compared comp) {
  yastl::pop_heap_aux(first, last - 1, last - 1, *(last - 1), distance_type(first), comp);
}

/*****************************************************************************************/
// sort_heap
// 该函数接受两个迭代器，表示 heap 容器的首尾，不断执行 pop_heap 操作，直到首尾最多相差1
/*****************************************************************************************/
template <class RandomIter>
void sort_heap(RandomIter first, RandomIter last) {
  // 每执行一次 pop_heap，最大的元素都被放到尾部，直到容器最多只有一个元素，完成排序
  while (last - first > 1) {
    yastl::pop_heap(first, last--);
  }
}

// 重载版本使用函数对象 comp 代替比较操作
template <class RandomIter, class Compared>
void sort_heap(RandomIter first, RandomIter last, Compared comp) {
  while (last - first > 1) {
    yastl::pop_heap(first, last--, comp);
  }
}

/*****************************************************************************************/
// make_heap
// 该函数接受两个迭代器，表示 heap 容器的首尾，把容器内的数据变为一个 heap
/*****************************************************************************************/
template <class RandomIter, class Distance>
void make_heap_aux(RandomIter first, RandomIter last, Distance*)
{
  if (last - first < 2)
    return;
  auto len = last - first;
  auto holeIndex = (len - 2) / 2; // 从最后一个非叶子节点依次向上调用heapify
  while (true) {
    // 重排以 holeIndex 为首的子树
    yastl::adjust_heap(first, holeIndex, len, *(first + holeIndex)); // 调用heapify
    if (holeIndex == 0) {
      return;
    }
    holeIndex--;
  }
}

template <class RandomIter>
void make_heap(RandomIter first, RandomIter last) {
  yastl::make_heap_aux(first, last, distance_type(first));
}

// 重载版本使用函数对象 comp 代替比较操作, 建立堆
template <class RandomIter, class Distance, class Compared>
void make_heap_aux(RandomIter first, RandomIter last, Distance*, Compared comp) {
  if (last - first < 2) { // 迭代器范围只有一个元素
    return;
  }
  auto len = last - first;
  auto holeIndex = (len - 2) / 2; // 从第一个非叶子节点开始，依次递减的调用heapify(adjust_heap)
  while (true) {
    // 重排以 holeIndex 为首的子树
    yastl::adjust_heap(first, holeIndex, len, *(first + holeIndex), comp); // heapify操作
    if (holeIndex == 0) { // 到根节点
      return;
    }
    holeIndex--;
  }
}

// 在迭代器[first, last)范围内建立堆
template <class RandomIter, class Compared>
void make_heap(RandomIter first, RandomIter last, Compared comp) {
  yastl::make_heap_aux(first, last, distance_type(first), comp);
}

} // namespace yastl
#endif // _INCLUDE_HEAP_ALGO_H_

