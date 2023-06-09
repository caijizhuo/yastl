﻿#ifndef _INCLUDE_HASHTABLE_H_
#define _INCLUDE_HASHTABLE_H_

// 这个头文件包含了一个模板类 hashtable
// hashtable : 哈希表，使用开链法处理冲突

#include <initializer_list>

#include "algo.h"
#include "functional.h"
#include "memory.h"
#include "vector.h"
#include "util.h"
#include "exceptdef.h"

namespace yastl {
/* hash table ____________________________
              | 1 | 2 | 3 | 4 | 5 | 6 | 7 |=> 每一个 ht->bucket
              ————————————————————————————
                    |
                    v
                  _______     _______
hash table node   |value|     |value|
                  -------     -------
                  |next | =>  |next |
                  ——————      ——————

*/
// hashtable 的节点定义
template <class T>
struct hashtable_node {
  hashtable_node* next;   // 指向下一个节点
  T value;  // 储存实值

  hashtable_node() = default;
  hashtable_node(const T& n) : next(nullptr), value(n) {}
  // 拷贝构造
  hashtable_node(const hashtable_node& node) : next(node.next), value(node.value) {}
  // 移动构造
  hashtable_node(hashtable_node&& node) : next(node.next), value(yastl::move(node.value)) {
    node.next = nullptr; // 防止 double free
  }
};

// value traits
// 给 unordered_set 用的，元素不是键值对
template <class T, bool>
struct ht_value_traits_imp {
  typedef T key_type;
  typedef T mapped_type; // 不用键值对，数据类型都为 T
  typedef T value_type; // 不用键值对，数据类型都为 T

  template <class Ty>
  static const key_type& get_key(const Ty& value) {
    return value;
  }

  template <class Ty>
  static const value_type& get_value(const Ty& value) {
    return value;
  }
};

// 给 unordered_map 用的，元素是键值对
template <class T>
struct ht_value_traits_imp<T, true> {
  typedef typename std::remove_cv<typename T::first_type>::type key_type; // 用键值对，数据类型为 T
  typedef typename T::second_type mapped_type; // 用键值对，数据类型为 mapped_type T::second_type
  typedef T value_type;

  template <class Ty>
  static const key_type& get_key(const Ty& value) {
    return value.first;
  }

  template <class Ty>
  static const value_type& get_value(const Ty& value) {
    return value;
  }
};

// 用来区分是键值对还是普通元素
template <class T>
struct ht_value_traits {
  static constexpr bool is_map = yastl::is_pair<T>::value;

  typedef ht_value_traits_imp<T, is_map> value_traits_type;

  typedef typename value_traits_type::key_type key_type;
  typedef typename value_traits_type::mapped_type mapped_type;
  typedef typename value_traits_type::value_type value_type;

  template <class Ty>
  static const key_type& get_key(const Ty& value) {
    return value_traits_type::get_key(value);
  }

  template <class Ty>
  static const value_type& get_value(const Ty& value) {
    return value_traits_type::get_value(value);
  }
};


// forward declaration

template <class T, class HashFun, class KeyEqual>
class hashtable;

template <class T, class HashFun, class KeyEqual>
struct ht_iterator;

template <class T, class HashFun, class KeyEqual>
struct ht_const_iterator;

template <class T>
struct ht_local_iterator;

template <class T>
struct ht_const_local_iterator;

// ht_iterator

template <class T, class Hash, class KeyEqual>
struct ht_iterator_base : public yastl::iterator<yastl::forward_iterator_tag, T> {
  typedef yastl::hashtable<T, Hash, KeyEqual> hashtable;
  typedef ht_iterator_base<T, Hash, KeyEqual> base;
  typedef yastl::ht_iterator<T, Hash, KeyEqual> iterator;
  typedef yastl::ht_const_iterator<T, Hash, KeyEqual> const_iterator;
  typedef hashtable_node<T>* node_ptr;
  typedef hashtable* contain_ptr;
  typedef const node_ptr const_node_ptr;
  typedef const contain_ptr const_contain_ptr;

  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  node_ptr node;  // 迭代器当前所指节点,指向一个 hashtable node
  contain_ptr ht;    // 保持与容器的连结，指向这个 node 所在的 hashtable

  ht_iterator_base() = default;

  bool operator==(const base& rhs) const {
    return node == rhs.node;
  }
  bool operator!=(const base& rhs) const {
    return node != rhs.node;
  }
};

template <class T, class Hash, class KeyEqual>
struct ht_iterator : public ht_iterator_base<T, Hash, KeyEqual> {
  typedef ht_iterator_base<T, Hash, KeyEqual> base;
  typedef typename base::hashtable hashtable;
  typedef typename base::iterator iterator;
  typedef typename base::const_iterator const_iterator;
  typedef typename base::node_ptr node_ptr;
  typedef typename base::contain_ptr contain_ptr; // 指向 hashtable 的指针

  typedef ht_value_traits<T> value_traits;
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;

  using base::node;
  using base::ht;

  // 构造函数
  ht_iterator() = default;
  ht_iterator(node_ptr n, contain_ptr t) {
    node = n;
    ht = t;
  }
  ht_iterator(const iterator& rhs) {
    node = rhs.node;
    ht = rhs.ht;
  }
  ht_iterator(const const_iterator& rhs) {
    node = rhs.node;
    ht = rhs.ht;
  }
  // 赋值运算符
  iterator& operator=(const iterator& rhs) {
    if (this != &rhs) {
      node = rhs.node;
      ht = rhs.ht;
    }
    return *this;
  }
  iterator& operator=(const const_iterator& rhs) {
    if (this != &rhs) {
      node = rhs.node;
      ht = rhs.ht;
    }
    return *this;
  }

  // 重载操作符
  reference operator*() const {
    return node->value;
  }
  // 返回 node->value 的地址
  pointer operator->() const {
    return &(operator*());
  }
  // ++i 遍历同一个 bucket 上的下一个 hashnode，若为空就跳到下一个 bucket
  iterator& operator++() {
    YASTL_DEBUG(node != nullptr);
    const node_ptr old = node;
    node = node->next;
    if (node == nullptr) { // 如果下一个位置为空，跳到下一个 bucket 的起始处
      auto index = ht->hash(value_traits::get_key(old->value)); // 拿到当前值的 bucket 编号
      while (!node && ++index < ht->bucket_size_) { // index + 1 后，将 node 设置为下一个 bucket 的第一个 hashnode
        node = ht->buckets_[index];
      }
    }
    return *this;
  }
  // ++i
  iterator operator++(int) {
    iterator tmp = *this;
    ++*this;
    return tmp;
  }
};

// 是 const 的指针
template <class T, class Hash, class KeyEqual>
struct ht_const_iterator : public ht_iterator_base<T, Hash, KeyEqual> {
  typedef ht_iterator_base<T, Hash, KeyEqual> base;
  typedef typename base::hashtable hashtable;
  typedef typename base::iterator iterator;
  typedef typename base::const_iterator const_iterator;
  typedef typename base::const_node_ptr node_ptr;
  typedef typename base::const_contain_ptr contain_ptr;

  typedef ht_value_traits<T> value_traits;
  typedef T value_type;
  typedef const value_type* pointer;
  typedef const value_type& reference;

  using base::node; // 当前 hashnode
  using base::ht; // 所在的 hashtable

  ht_const_iterator() = default;
  ht_const_iterator(node_ptr n, contain_ptr t) {
    node = n;
    ht = t;
  }
  ht_const_iterator(const iterator& rhs) {
    node = rhs.node;
    ht = rhs.ht;
  }
  ht_const_iterator(const const_iterator& rhs) {
    node = rhs.node;
    ht = rhs.ht;
  }
  const_iterator& operator=(const iterator& rhs) {
    if (this != &rhs) {
      node = rhs.node;
      ht = rhs.ht;
    }
    return *this;
  }
  const_iterator& operator=(const const_iterator& rhs) {
    if (this != &rhs) {
      node = rhs.node;
      ht = rhs.ht;
    }
    return *this;
  }

  // 重载操作符
  reference operator*() const {
    return node->value;
  }
  pointer operator->() const {
    return &(operator*());
  }

  const_iterator& operator++() {
    YASTL_DEBUG(node != nullptr);
    const node_ptr old = node;
    node = node->next;
    if (node == nullptr) { // 如果下一个位置为空，跳到下一个 bucket 的起始处
      auto index = ht->hash(value_traits::get_key(old->value));
      while (!node && ++index < ht->bucket_size_) {
        node = ht->buckets_[index]; // 换到下一个首地址
      }
    }
    return *this;
  }
  const_iterator operator++(int) {
    const_iterator tmp = *this;
    ++*this;
    return tmp;
  }
};

// local iterator, 只在某个 bucket 中使用
template <class T>
struct ht_local_iterator : public yastl::iterator<yastl::forward_iterator_tag, T> {
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type& reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef hashtable_node<T>* node_ptr;

  typedef ht_local_iterator<T> self;
  typedef ht_local_iterator<T> local_iterator;
  typedef ht_const_local_iterator<T> const_local_iterator;
  node_ptr node;

  ht_local_iterator(node_ptr n) : node(n) {}
  ht_local_iterator(const local_iterator& rhs) : node(rhs.node) {}
  ht_local_iterator(const const_local_iterator& rhs) : node(rhs.node) {}

  reference operator*() const {
    return node->value;
  }
  pointer operator->() const {
    return &(operator*());
  }
  // ++it
  self& operator++() { // 只返回 node 的下一个值，不需判断是不是链的最后一个
    YASTL_DEBUG(node != nullptr);
    node = node->next;
    return *this;
  }
  // it++
  self operator++(int) {
    self tmp(*this);
    ++*this;
    return tmp;
  }

  bool operator==(const self& other) const {
    return node == other.node;
  }
  bool operator!=(const self& other) const {
    return node != other.node;
  }
};

// 只在某个 bucket 中使用
template <class T>
struct ht_const_local_iterator : public yastl::iterator<yastl::forward_iterator_tag, T> {
  typedef T value_type;
  typedef const value_type* pointer;
  typedef const value_type& reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef const hashtable_node<T>* node_ptr;

  typedef ht_const_local_iterator<T> self;
  typedef ht_local_iterator<T> local_iterator;
  typedef ht_const_local_iterator<T> const_local_iterator;

  node_ptr node;

  ht_const_local_iterator(node_ptr n) : node(n) {}
  ht_const_local_iterator(const local_iterator& rhs) : node(rhs.node) {}
  ht_const_local_iterator(const const_local_iterator& rhs) : node(rhs.node) {}

  reference operator*() const { // 返回所指向的 hashnode 的值
    return node->value;
  }
  pointer operator->() const {
    return &(operator*());
  }

  self& operator++() { // 只返回 node 的下一个值，不需判断是不是链的最后一个
    YASTL_DEBUG(node != nullptr);
    node = node->next;
    return *this;
  }

  self operator++(int) {
    self tmp(*this);
    ++*this;
    return tmp;
  }

  bool operator==(const self& other) const {
    return node == other.node;
  }
  bool operator!=(const self& other) const {
    return node != other.node;
  }
};

// bucket 使用的大小

#if (_MSC_VER && _WIN64) || ((__GNUC__ || __clang__) &&__SIZEOF_POINTER__ == 8)
#define SYSTEM_64 1
#else
#define SYSTEM_32 1
#endif

#ifdef SYSTEM_64

#define PRIME_NUM 99

// 1. start with p = 101
// 2. p = next_prime(p * 1.7)
// 3. if p < (2 << 63), go to step 2, otherwise, go to step 4
// 4. end with p = prev_prime(2 << 63 - 1)
static constexpr size_t ht_prime_list[] = {
  101ull, 173ull, 263ull, 397ull, 599ull, 907ull, 1361ull, 2053ull, 3083ull,
  4637ull, 6959ull, 10453ull, 15683ull, 23531ull, 35311ull, 52967ull, 79451ull,
  119179ull, 178781ull, 268189ull, 402299ull, 603457ull, 905189ull, 1357787ull,
  2036687ull, 3055043ull, 4582577ull, 6873871ull, 10310819ull, 15466229ull,
  23199347ull, 34799021ull, 52198537ull, 78297827ull, 117446801ull, 176170229ull,
  264255353ull, 396383041ull, 594574583ull, 891861923ull, 1337792887ull,
  2006689337ull, 3010034021ull, 4515051137ull, 6772576709ull, 10158865069ull,
  15238297621ull, 22857446471ull, 34286169707ull, 51429254599ull, 77143881917ull,
  115715822899ull, 173573734363ull, 260360601547ull, 390540902329ull, 
  585811353559ull, 878717030339ull, 1318075545511ull, 1977113318311ull, 
  2965669977497ull, 4448504966249ull, 6672757449409ull, 10009136174239ull,
  15013704261371ull, 22520556392057ull, 33780834588157ull, 50671251882247ull,
  76006877823377ull, 114010316735089ull, 171015475102649ull, 256523212653977ull,
  384784818980971ull, 577177228471507ull, 865765842707309ull, 1298648764060979ull,
  1947973146091477ull, 2921959719137273ull, 4382939578705967ull, 6574409368058969ull,
  9861614052088471ull, 14792421078132871ull, 22188631617199337ull, 33282947425799017ull,
  49924421138698549ull, 74886631708047827ull, 112329947562071807ull, 168494921343107851ull,
  252742382014661767ull, 379113573021992729ull, 568670359532989111ull, 853005539299483657ull,
  1279508308949225477ull, 1919262463423838231ull, 2878893695135757317ull, 4318340542703636011ull,
  6477510814055453699ull, 9716266221083181299ull, 14574399331624771603ull, 18446744073709551557ull
};

#else

#define PRIME_NUM 44

// 1. start with p = 101
// 2. p = next_prime(p * 1.7)
// 3. if p < (2 << 31), go to step 2, otherwise, go to step 4
// 4. end with p = prev_prime(2 << 31 - 1)
static constexpr size_t ht_prime_list[] = {
  101u, 173u, 263u, 397u, 599u, 907u, 1361u, 2053u, 3083u, 4637u, 6959u, 
  10453u, 15683u, 23531u, 35311u, 52967u, 79451u, 119179u, 178781u, 268189u,
  402299u, 603457u, 905189u, 1357787u, 2036687u, 3055043u, 4582577u, 6873871u,
  10310819u, 15466229u, 23199347u, 34799021u, 52198537u, 78297827u, 117446801u,
  176170229u, 264255353u, 396383041u, 594574583u, 891861923u, 1337792887u,
  2006689337u, 3010034021u, 4294967291u,
};

#endif

// 找出最接近并 >=n 的那个质数
inline size_t ht_next_prime(size_t n) {
  const size_t* first = ht_prime_list;
  const size_t* last = ht_prime_list + PRIME_NUM;
  const size_t* pos = yastl::lower_bound(first, last, n);
  return pos == last ? *(last - 1) : *pos; // 找不到就用最大的，否则就用最接近的
}

// 模板类 hashtable
// 参数一代表数据类型，参数二代表哈希函数，参数三代表键值相等的比较函数
template <class T, class Hash, class KeyEqual>
class hashtable {

  friend struct yastl::ht_iterator<T, Hash, KeyEqual>;
  friend struct yastl::ht_const_iterator<T, Hash, KeyEqual>;

public:
  // hashtable 的型别定义
  typedef ht_value_traits<T> value_traits;
  typedef typename value_traits::key_type key_type;
  typedef typename value_traits::mapped_type mapped_type;
  typedef typename value_traits::value_type value_type;
  typedef Hash hasher;
  typedef KeyEqual key_equal;

  typedef hashtable_node<T> node_type;
  typedef node_type* node_ptr;
  typedef yastl::vector<node_ptr> bucket_type; // 桶的类别 vector，元素为指向 hashnode 的指针

  typedef yastl::allocator<T> allocator_type;
  typedef yastl::allocator<T> data_allocator;
  typedef yastl::allocator<node_type> node_allocator;

  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;

  typedef yastl::ht_iterator<T, Hash, KeyEqual> iterator;
  typedef yastl::ht_const_iterator<T, Hash, KeyEqual> const_iterator;
  typedef yastl::ht_local_iterator<T> local_iterator;
  typedef yastl::ht_const_local_iterator<T> const_local_iterator;

  allocator_type get_allocator() const {
    return allocator_type();
  }

private:
  // 用以下六个参数来表现 hashtable
  bucket_type buckets_; // 桶的 vector
  size_type bucket_size_; // 桶的数量
  size_type size_; // 元素个数
  float mlf_; // max load factor 最大负载系数
  hasher hash_; // 哈希函数
  key_equal equal_; // 键值相等的比较函数

private:
  bool is_equal(const key_type& key1, const key_type& key2) {
    return equal_(key1, key2);
  }

  bool is_equal(const key_type& key1, const key_type& key2) const {
    return equal_(key1, key2);
  }

  // change const iterator 把 node 强制转换为指向 hashtable 的 const 指针
  const_iterator M_cit(node_ptr node) const noexcept {
    return const_iterator(node, const_cast<hashtable*>(this));
  }

  // 找到第一个有节点的位置就返回
  iterator M_begin() noexcept {
    for (size_type n = 0; n < bucket_size_; ++n) {
      if (buckets_[n]) {
        return iterator(buckets_[n], this);
      }
    }
    return iterator(nullptr, this);
  }

  const_iterator M_begin() const noexcept {
    for (size_type n = 0; n < bucket_size_; ++n) {
      if (buckets_[n]) { // 找到第一个有节点的位置就返回
        return M_cit(buckets_[n]);
      }
    }
    return M_cit(nullptr);
  }

public:
  // 构造、复制、移动、析构函数

  // 构造函数
  explicit hashtable(size_type bucket_count, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual())
    : size_(0), mlf_(1.0f), hash_(hash), equal_(equal) {
    init(bucket_count);
  }

  // 调用迭代器构造
  template <class Iter, typename std::enable_if<yastl::is_input_iterator<Iter>::value, int>::type = 0>
    hashtable(Iter first, Iter last, size_type bucket_count, const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual())
    : size_(yastl::distance(first, last)), mlf_(1.0f), hash_(hash), equal_(equal) {
    init(yastl::max(bucket_count, static_cast<size_type>(yastl::distance(first, last))));
  }

  // 拷贝构造
  hashtable(const hashtable& rhs) : hash_(rhs.hash_), equal_(rhs.equal_) {
    copy_init(rhs);
  }

  // 移动构造
  hashtable(hashtable&& rhs) noexcept : bucket_size_(rhs.bucket_size_), 
    size_(rhs.size_),
    mlf_(rhs.mlf_),
    hash_(rhs.hash_),
    equal_(rhs.equal_) {
    buckets_ = yastl::move(rhs.buckets_);
    rhs.bucket_size_ = 0;
    rhs.size_ = 0;
    rhs.mlf_ = 0.0f;
  }

  hashtable& operator=(const hashtable& rhs);
  hashtable& operator=(hashtable&& rhs) noexcept;

  ~hashtable() {
    clear();
  }

  // 迭代器相关操作
  iterator begin() noexcept {
    return M_begin();
  }
  const_iterator begin() const noexcept {
    return M_begin();
  }
  iterator end() noexcept {
    return iterator(nullptr, this);
  }
  const_iterator end() const noexcept {
    return M_cit(nullptr);
  }
  
  const_iterator cbegin() const noexcept {
    return begin();
  }
  const_iterator cend() const noexcept {
    return end();
  }

  // 容量相关操作
  bool empty() const noexcept {
    return size_ == 0;
  }
  size_type size() const noexcept {
    return size_;
  }
  size_type max_size() const noexcept {
    return static_cast<size_type>(-1);
  }

  // 修改容器相关操作

  // emplace / empalce_hint

  template <class ...Args>
  iterator emplace_multi(Args&& ...args);

  template <class ...Args>
  pair<iterator, bool> emplace_unique(Args&& ...args);

  // [note]: hint 对于 hash_table 其实没有意义，因为即使提供了 hint，也要做一次 hash，
  // 来确保 hash_table 的性质，所以选择忽略它
  template <class ...Args>
  iterator emplace_multi_use_hint(const_iterator /*hint*/, Args&& ...args) {
    return emplace_multi(yastl::forward<Args>(args)...);
  }

  template <class ...Args>
  iterator emplace_unique_use_hint(const_iterator /*hint*/, Args&& ...args) {
    return emplace_unique(yastl::forward<Args>(args)...).first;
  }

  // insert
  // 不需要重新 hash 的方式
  iterator insert_multi_noresize(const value_type& value);
  pair<iterator, bool> insert_unique_noresize(const value_type& value);

  iterator insert_multi(const value_type& value) {
    rehash_if_need(1); // 增加一个单位 判断是否需要重新分配空间
    return insert_multi_noresize(value);
  }

  iterator insert_multi(value_type&& value) {
    return emplace_multi(yastl::move(value));
  }

  pair<iterator, bool> insert_unique(const value_type& value) {
    rehash_if_need(1);
    return insert_unique_noresize(value);
  }

  pair<iterator, bool> insert_unique(value_type&& value) {
    return emplace_unique(yastl::move(value));
  }

  // [note]: 同 emplace_hint
  iterator insert_multi_use_hint(const_iterator /*hint*/, const value_type& value) {
    return insert_multi(value);
  }
  // 移动构造 value
  iterator insert_multi_use_hint(const_iterator /*hint*/, value_type&& value) {
    return emplace_multi(yastl::move(value));
  }

  iterator insert_unique_use_hint(const_iterator /*hint*/, const value_type& value) {
    return insert_unique(value).first;
  }
  iterator insert_unique_use_hint(const_iterator /*hint*/, value_type&& value) {
    return emplace_unique(yastl::move(value));
  }

  // 插入[first, last) 内的元素到 hashtable 中，允许重复
  template <class InputIter>
  void insert_multi(InputIter first, InputIter last) {
    copy_insert_multi(first, last, iterator_category(first));
  }

  // 插入[first, last) 内的元素到 hashtable 中，不允许重复
  template <class InputIter>
  void insert_unique(InputIter first, InputIter last) {
    copy_insert_unique(first, last, iterator_category(first));
  }

  // erase / clear

  void erase(const_iterator position);
  void erase(const_iterator first, const_iterator last);

  // 在允许重复的的 hashtable 中删除 key
  size_type erase_multi(const key_type& key);
  // 在允许重复的的 hashtable 中删除 key
  size_type erase_unique(const key_type& key);

  void clear();

  void swap(hashtable& rhs) noexcept;

  // 查找相关操作

  size_type count(const key_type& key) const;

  iterator find(const key_type& key);
  const_iterator find(const key_type& key) const;

  pair<iterator, iterator> equal_range_multi(const key_type& key);
  pair<const_iterator, const_iterator> equal_range_multi(const key_type& key) const;

  pair<iterator, iterator> equal_range_unique(const key_type& key);
  pair<const_iterator, const_iterator> equal_range_unique(const key_type& key) const;

  // bucket interface
  // 返回第 n 个桶的第一个 hashnode 的迭代器
  local_iterator begin(size_type n) noexcept {
    YASTL_DEBUG(n < size_);
    return buckets_[n];
  }
  const_local_iterator begin(size_type n)  const noexcept {
    YASTL_DEBUG(n < size_);
    return buckets_[n];
  }
  const_local_iterator cbegin(size_type n) const noexcept {
    YASTL_DEBUG(n < size_);
    return buckets_[n];
  }

  // 返回 nullptr
  local_iterator end(size_type n) noexcept {
    YASTL_DEBUG(n < size_);
    return nullptr;
  }
  // 返回 nullptr
  const_local_iterator end(size_type n) const noexcept {
    YASTL_DEBUG(n < size_);
    return nullptr; 
  }
  // 返回 const nullptr
  const_local_iterator cend(size_type n) const noexcept {
    YASTL_DEBUG(n < size_);
    return nullptr; 
  }

  // 现有桶的数量
  size_type bucket_count() const noexcept {
    return bucket_size_;
  }
  // 最大可配置的桶的数量
  size_type max_bucket_count() const noexcept {
    return ht_prime_list[PRIME_NUM - 1];
  }

  size_type bucket_size(size_type n) const noexcept;
  // 返回 key 应该所在的 bucket 的索引(size_type)
  size_type bucket(const key_type& key) const {
    return hash(key);
  }

  // hash policy
  // 负载系数 为元素个数 / 桶的个数
  float load_factor() const noexcept {
    return bucket_size_ != 0 ? (float)size_ / bucket_size_ : 0.0f;
  }

  // 一个阈值，返回 mlf_，所能接受的最大负载系数
  float max_load_factor() const noexcept {
    return mlf_;
  }
  // 设置最大负载系数
  void max_load_factor(float ml) {
    THROW_OUT_OF_RANGE_IF(ml != ml || ml < 0, "invalid hash load factor");
    mlf_ = ml;
  }
  // 重新 hash count 个 bucket
  void rehash(size_type count);

  // 预留 count 个 hashnode
  void reserve(size_type count) {
    rehash(static_cast<size_type>((float)count / max_load_factor() + 0.5f)); // （不知道这数哪来的）
  }

  hasher hash_fcn() const {
    return hash_;
  }

  key_equal key_eq() const {
    return equal_;
  }

private:
  // hashtable 成员函数

  // init
  void init(size_type n);
  void copy_init(const hashtable& ht);

  // node
  template <class ...Args>
  node_ptr create_node(Args&& ...args);
  void destroy_node(node_ptr n);

  // hash
  size_type next_size(size_type n) const;
  size_type hash(const key_type& key, size_type n) const;
  size_type hash(const key_type& key) const;
  void rehash_if_need(size_type n);

  // insert
  template <class InputIter>
  void copy_insert_multi(InputIter first, InputIter last, yastl::input_iterator_tag);
  template <class ForwardIter>
  void copy_insert_multi(ForwardIter first, ForwardIter last, yastl::forward_iterator_tag);
  template <class InputIter>
  void copy_insert_unique(InputIter first, InputIter last, yastl::input_iterator_tag);
  template <class ForwardIter>
  void copy_insert_unique(ForwardIter first, ForwardIter last, yastl::forward_iterator_tag);

  // insert node
  pair<iterator, bool> insert_node_unique(node_ptr np);
  iterator insert_node_multi(node_ptr np);

  // bucket operator
  void replace_bucket(size_type bucket_count);
  void erase_bucket(size_type n, node_ptr first, node_ptr last);
  void erase_bucket(size_type n, node_ptr last);

  // comparision
  bool equal_to_multi(const hashtable& other);
  bool equal_to_unique(const hashtable& other);
};

/*****************************************************************************************/

// 复制赋值运算符
template <class T, class Hash, class KeyEqual>
hashtable<T, Hash, KeyEqual>&
hashtable<T, Hash, KeyEqual>::operator=(const hashtable& rhs) {
  if (this != &rhs) {
    hashtable tmp(rhs);
    swap(tmp);
  }
  return *this;
}

// 移动赋值运算符
template <class T, class Hash, class KeyEqual>
hashtable<T, Hash, KeyEqual>&
hashtable<T, Hash, KeyEqual>::operator=(hashtable&& rhs) noexcept {
  hashtable tmp(yastl::move(rhs));
  swap(tmp);
  return *this;
}

// 就地构造元素，键值允许重复
// 强异常安全保证
template <class T, class Hash, class KeyEqual>
template <class ...Args>
typename hashtable<T, Hash, KeyEqual>::iterator
hashtable<T, Hash, KeyEqual>::emplace_multi(Args&& ...args) {
  auto np = create_node(yastl::forward<Args>(args)...);
  try {
    if ((float)(size_ + 1) > (float)bucket_size_ * max_load_factor()) { // 元素个数超过负载数
      rehash(size_ + 1);
    }
  } catch (...) {
    destroy_node(np);
    throw;
  }
  return insert_node_multi(np); // 允许重复
}

// 就地构造元素，键值允许重复
// 强异常安全保证
template <class T, class Hash, class KeyEqual>
template <class ...Args>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, bool> 
hashtable<T, Hash, KeyEqual>::emplace_unique(Args&& ...args) {
  auto np = create_node(yastl::forward<Args>(args)...);
  try {
    if ((float)(size_ + 1) > (float)bucket_size_ * max_load_factor()) {
      rehash(size_ + 1);
    }
  } catch (...) {
    destroy_node(np);
    throw;
  }
  return insert_node_unique(np);
}

// 在不需要重建表格的情况下插入新节点，键值不允许重复，返回已有的迭代器或者插入的迭代器以及状态的 pair
template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, bool>
hashtable<T, Hash, KeyEqual>::insert_unique_noresize(const value_type& value) {
  const auto n = hash(value_traits::get_key(value));
  auto first = buckets_[n];
  for (auto cur = first; cur; cur = cur->next) {
    if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(value))) {
      return yastl::make_pair(iterator(cur, this), false); // 如果已经存在，直接返回，并且状态标记为 false
    }
  }
  // 让新节点成为链表的第一个节点
  auto tmp = create_node(value);  
  tmp->next = first; // 插入头部
  buckets_[n] = tmp;
  ++size_;
  return yastl::make_pair(iterator(tmp, this), true); // 不存在，插入，状态返回 true
}

// 在不需要重建表格的情况下插入新节点，键值允许重复，返回插入的 hashnode 的迭代器
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::iterator
hashtable<T, Hash, KeyEqual>::insert_multi_noresize(const value_type& value) {
  const auto n = hash(value_traits::get_key(value));
  auto first = buckets_[n];
  auto tmp = create_node(value);
  for (auto cur = first; cur; cur = cur->next) {
    // 如果链表中存在相同键值的节点就马上插入，然后返回
    if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(value))) {
      tmp->next = cur->next; // 存在相同键值，插在它后面
      cur->next = tmp;
      ++size_;
      return iterator(tmp, this);
    }
  }
  // 否则插入在链表头部
  tmp->next = first;
  buckets_[n] = tmp;
  ++size_;
  return iterator(tmp, this);
}

// 删除迭代器所指的节点
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::erase(const_iterator position) {
  auto p = position.node;
  if (p) {
    const auto n = hash(value_traits::get_key(p->value));
    auto cur = buckets_[n]; // cur 为链表头部的 hashnode
    if (cur == p) { // p 位于链表头部
      buckets_[n] = cur->next;
      destroy_node(cur);
      --size_;
    } else {
      auto next = cur->next;
      while (next) {
        if (next == p) { // next 为要删除的节点
          cur->next = next->next;
          destroy_node(next);
          --size_;
          break;
        } else { // 向后移动
          cur = next;
          next = cur->next;
        }
      }
    }
  }
}

// 删除 [first, last) 内的节点
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::erase(const_iterator first, const_iterator last) {
  if (first.node == last.node) {
    return;
  }
  auto first_bucket = first.node
    ? hash(value_traits::get_key(first.node->value)) 
    : bucket_size_;
  auto last_bucket = last.node 
    ? hash(value_traits::get_key(last.node->value))
    : bucket_size_;
  if (first_bucket == last_bucket) { // 如果在 bucket 在同一个位置, 迭代器在同一条链上
    erase_bucket(first_bucket, first.node, last.node);
  } else { // 迭代器跨了几个 bucket
    erase_bucket(first_bucket, first.node, nullptr); // 将 first bucket 链表全部删除
    for (auto n = first_bucket + 1; n < last_bucket; ++n) { // 清空 (first, last) 所有 bucket 的链表
      if (buckets_[n] != nullptr) {
        erase_bucket(n, nullptr);
      }
    }
    if (last_bucket != bucket_size_) { // 清空 last 的 bucket 中 直到 last.node 的链表
      erase_bucket(last_bucket, last.node);
    }
  }
}

// 删除键值为 key 的节点，返回删除的个数 (size_type)
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::erase_multi(const key_type& key) {
  auto p = equal_range_multi(key);
  if (p.first.node != nullptr) {
    erase(p.first, p.second);
    return yastl::distance(p.first, p.second);
  }
  return 0;
}

// 删除 key 所在 node (存在唯一)，返回删除个数(size_type)
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::erase_unique(const key_type& key) {
  const auto n = hash(key);
  auto first = buckets_[n];
  if (first) { // 链表首节点
    if (is_equal(value_traits::get_key(first->value), key)) { // 是第一个，删除并改变头节点
      buckets_[n] = first->next;
      destroy_node(first);
      --size_;
      return 1;
    } else {
      auto next = first->next;
      while (next) { // 遍历找到并删除
        if (is_equal(value_traits::get_key(next->value), key)) {
          first->next = next->next;
          destroy_node(next);
          --size_;
          return 1;
        }
        first = next;
        next = first->next;
      }
    }
  }
  return 0;
}

// 清空 hashtable
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::clear() {
  if (size_ != 0) {
    for (size_type i = 0; i < bucket_size_; ++i) { // 遍历每个 bucket
      node_ptr cur = buckets_[i];
      while (cur != nullptr) { // 清空每个 bucket 的 node
        node_ptr next = cur->next;
        destroy_node(cur);
        cur = next;
      }
      buckets_[i] = nullptr;
    }
    size_ = 0;
  }
}

// 在某个 bucket 节点的个数
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::bucket_size(size_type n) const noexcept {
  size_type result = 0;
  for (auto cur = buckets_[n]; cur; cur = cur->next) { // 遍历第 n 个 bucket 的每个链表，数个数
    ++result;
  }
  return result;
}

// 重新对元素进行一遍哈希，插入到新的位置。count 为元素个数
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::rehash(size_type count) {
  auto n = ht_next_prime(count); // >=元素个数的最小质数
  if (n > bucket_size_) { // 比现有的大小大
    replace_bucket(n);
  } else { // 比现有大小小
    if (((float)size_ / (float)n < max_load_factor() - 0.25f) &&  // 调整后最大负载系数比现在还低 0.25 并且
        ((float)n < (float)bucket_size_ * 0.75)) { // 新的大小比现有大小的 3/4 还小才值得重新调整
      replace_bucket(n);
    }
  }
}

// 查找键值为 key 的节点，返回其迭代器
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::iterator
hashtable<T, Hash, KeyEqual>::find(const key_type& key) {
  const auto n = hash(key);
  node_ptr first = buckets_[n];
  // 依次遍历
  for (; first && !is_equal(value_traits::get_key(first->value), key); first = first->next) {}
  return iterator(first, this);
}

template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::const_iterator
hashtable<T, Hash, KeyEqual>::find(const key_type& key) const {
  const auto n = hash(key);
  node_ptr first = buckets_[n];
  // 依次遍历
  for (; first && !is_equal(value_traits::get_key(first->value), key); first = first->next) {}
  return M_cit(first);
}

// 查找键值为 key 出现的次数
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::count(const key_type& key) const {
  const auto n = hash(key);
  size_type result = 0;
  for (node_ptr cur = buckets_[n]; cur; cur = cur->next) { // 相同 key 必定出现在同一条链上
    if (is_equal(value_traits::get_key(cur->value), key)) {
      ++result;
    }
  }
  return result;
}

// 查找与键值 key 相等的区间，返回一个 pair，指向相等区间的首尾 [first, second)
template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, typename hashtable<T, Hash, KeyEqual>::iterator>
hashtable<T, Hash, KeyEqual>::equal_range_multi(const key_type& key) {
  const auto n = hash(key); // 找到 bucket 编号
  for (node_ptr first = buckets_[n]; first; first = first->next) { // 遍历 n 号 bucket
    if (is_equal(value_traits::get_key(first->value), key)) { // 如果出现相等的键值，记为 first
      for (node_ptr second = first->next; second; second = second->next) { // 尝试找到第二个相等的键值 second
        if (!is_equal(value_traits::get_key(second->value), key)) { // 找到第一个不等的，在他之前都相等
          return yastl::make_pair(iterator(first, this), iterator(second, this));
        }
      }
      for (auto m = n + 1; m < bucket_size_; ++m) { // 整个链表都相等，查找下一个链表出现的位置
        if (buckets_[m]) {
          return yastl::make_pair(iterator(first, this), iterator(buckets_[m], this));
        }
      }
      return yastl::make_pair(iterator(first, this), end()); // 下一个链表不存在
    }
  }
  return yastl::make_pair(end(), end()); // 根本就不存在 key
}

// 查找与键值 key 相等的区间，返回一个 pair，指向相等区间的首尾 [first, second)
template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::const_iterator, typename hashtable<T, Hash, KeyEqual>::const_iterator>
hashtable<T, Hash, KeyEqual>::equal_range_multi(const key_type& key) const {
  const auto n = hash(key);
  for (node_ptr first = buckets_[n]; first; first = first->next) {
    if (is_equal(value_traits::get_key(first->value), key)) {
      for (node_ptr second = first->next; second; second = second->next) {
        if (!is_equal(value_traits::get_key(second->value), key)) {
          return yastl::make_pair(M_cit(first), M_cit(second));
        }
      }
      for (auto m = n + 1; m < bucket_size_; ++m) { // 整个链表都相等，查找下一个链表出现的位置
        if (buckets_[m]) {
          return yastl::make_pair(M_cit(first), M_cit(buckets_[m]));
        }
      }
      return yastl::make_pair(M_cit(first), cend());
    }
  }
  return yastl::make_pair(cend(), cend());
}

// 查找与键值 key 相等(唯一)的区间，返回一个 pair，指向相等区间的首尾 [first, second)
template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, typename hashtable<T, Hash, KeyEqual>::iterator>
hashtable<T, Hash, KeyEqual>::equal_range_unique(const key_type& key) {
  const auto n = hash(key);
  for (node_ptr first = buckets_[n]; first; first = first->next) { // 遍历对应 bucket 的链
    if (is_equal(value_traits::get_key(first->value), key)) { // 找到了
      if (first->next) { // 下一个 node 不为空
        return yastl::make_pair(iterator(first, this), iterator(first->next, this));
      }
      for (auto m = n + 1; m < bucket_size_; ++m) { // 整个链表都相等，查找下一个链表出现的位置
        if (buckets_[m]) {
          return yastl::make_pair(iterator(first, this), iterator(buckets_[m], this)); // 有下一个链表
        }
      }
      return yastl::make_pair(iterator(first, this), end()); // key 所在链表为最后一个链表
    }
  }
  return yastl::make_pair(end(), end()); // 根本没有 key
}

// 查找与键值 key 相等(唯一)的区间，返回一个 pair，指向相等区间的首尾 [first, second)
template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::const_iterator,
  typename hashtable<T, Hash, KeyEqual>::const_iterator>
hashtable<T, Hash, KeyEqual>::equal_range_unique(const key_type& key) const {
  const auto n = hash(key);
  for (node_ptr first = buckets_[n]; first; first = first->next) {
    if (is_equal(value_traits::get_key(first->value), key)) {
      if (first->next) {
        return yastl::make_pair(M_cit(first), M_cit(first->next));
      }
      for (auto m = n + 1; m < bucket_size_; ++m) { // 整个链表都相等，查找下一个链表出现的位置
        if (buckets_[m]) {
          return yastl::make_pair(M_cit(first), M_cit(buckets_[m]));
        }
      }
      return yastl::make_pair(M_cit(first), cend());
    }
  }
  return yastl::make_pair(cend(), cend());
}

// 交换 hashtable
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::swap(hashtable& rhs) noexcept {
  if (this != &rhs) {
    buckets_.swap(rhs.buckets_);
    yastl::swap(bucket_size_, rhs.bucket_size_);
    yastl::swap(size_, rhs.size_);
    yastl::swap(mlf_, rhs.mlf_);
    yastl::swap(hash_, rhs.hash_);
    yastl::swap(equal_, rhs.equal_);
  }
}

/****************************************************************************************/
// helper function

// init 函数
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::init(size_type n) {
  const auto bucket_nums = next_size(n); // 找到一个 >= n 的质数
  try {
    buckets_.reserve(bucket_nums); // 分配空间
    buckets_.assign(bucket_nums, nullptr);
  } catch (...) {
    bucket_size_ = 0;
    size_ = 0;
    throw;
  }
  bucket_size_ = buckets_.size();
}

// copy_init 函数
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::copy_init(const hashtable& ht) {
  bucket_size_ = 0;
  buckets_.reserve(ht.bucket_size_);
  buckets_.assign(ht.bucket_size_, nullptr);
  try {
    for (size_type i = 0; i < ht.bucket_size_; ++i) {
      node_ptr cur = ht.buckets_[i];
      if (cur) { // 如果某 bucket 存在链表
        auto copy = create_node(cur->value);
        buckets_[i] = copy;
        for (auto next = cur->next; next; cur = next, next = cur->next) { // 遍历并复制链表
          copy->next = create_node(next->value);
          copy = copy->next;
        }
        copy->next = nullptr; // 最后的空指针
      }
    }
    bucket_size_ = ht.bucket_size_;
    mlf_ = ht.mlf_;
    size_ = ht.size_;
  } catch (...) {
    clear();
  }
}

// create_node 函数
template <class T, class Hash, class KeyEqual>
template <class ...Args>
typename hashtable<T, Hash, KeyEqual>::node_ptr
hashtable<T, Hash, KeyEqual>::create_node(Args&& ...args) {
  node_ptr tmp = node_allocator::allocate(1);
  try {
    data_allocator::construct(yastl::address_of(tmp->value), yastl::forward<Args>(args)...);
    tmp->next = nullptr;
  } catch (...) {
    node_allocator::deallocate(tmp);
    throw;
  }
  return tmp;
}

// destroy_node 函数， 调用析构函数并且释放节点内存
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::destroy_node(node_ptr node) {
  data_allocator::destroy(yastl::address_of(node->value));
  node_allocator::deallocate(node);
  node = nullptr;
}

// next_size 函数，返回 >= n 的下一个合适质数大小作为新的 bucket_size 用
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::next_size(size_type n) const {
  return ht_next_prime(n);
}

// hash 函数，调用 hash_ 函数后取余
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::hash(const key_type& key, size_type n) const {
  return hash_(key) % n;
}
// hash 函数，调用 hash_ 函数后取余
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::size_type
hashtable<T, Hash, KeyEqual>::hash(const key_type& key) const {
  return hash_(key) % bucket_size_;
}

// rehash_if_need 函数, 增加大小为 n，计算是否需要重新排布 hashtable
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::rehash_if_need(size_type n) {
  if (static_cast<float>(size_ + n) > (float)bucket_size_ * max_load_factor()) {
    rehash(size_ + n);
  }
}

// copy_insert
template <class T, class Hash, class KeyEqual>
template <class InputIter>
void hashtable<T, Hash, KeyEqual>::copy_insert_multi(InputIter first, InputIter last, yastl::input_iterator_tag) {
  rehash_if_need(yastl::distance(first, last)); // 插入总元素个数
  for (; first != last; ++first) {
    insert_multi_noresize(*first);
  }
}

template <class T, class Hash, class KeyEqual>
template <class ForwardIter>
void hashtable<T, Hash, KeyEqual>::
copy_insert_multi(ForwardIter first, ForwardIter last, yastl::forward_iterator_tag) {
  size_type n = yastl::distance(first, last);
  rehash_if_need(n);
  for (; n > 0; --n, ++first) {
    insert_multi_noresize(*first);
  }
}

template <class T, class Hash, class KeyEqual>
template <class InputIter>
void hashtable<T, Hash, KeyEqual>::
copy_insert_unique(InputIter first, InputIter last, yastl::input_iterator_tag) {
  rehash_if_need(yastl::distance(first, last));
  for (; first != last; ++first) {
    insert_unique_noresize(*first);
  }
}

template <class T, class Hash, class KeyEqual>
template <class ForwardIter>
void hashtable<T, Hash, KeyEqual>::
copy_insert_unique(ForwardIter first, ForwardIter last, yastl::forward_iterator_tag) {
  size_type n = yastl::distance(first, last);
  rehash_if_need(n);
  for (; n > 0; --n, ++first) {
    insert_unique_noresize(*first);
  }
}

// insert_node 函数 返回插入的迭代器
template <class T, class Hash, class KeyEqual>
typename hashtable<T, Hash, KeyEqual>::iterator
hashtable<T, Hash, KeyEqual>::insert_node_multi(node_ptr np) {
  const auto n = hash(value_traits::get_key(np->value));
  auto cur = buckets_[n];
  if (cur == nullptr) { // 还是空的 放在头部
    buckets_[n] = np;
    ++size_;
    return iterator(np, this);
  }
  for (; cur; cur = cur->next) {
    if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(np->value))) { // 相等插在后面
      np->next = cur->next;
      cur->next = np;
      ++size_;
      return iterator(np, this);
    }
  }
  np->next = buckets_[n]; // 插在头部
  buckets_[n] = np;
  ++size_;
  return iterator(np, this);
}

// insert_node_unique 函数
template <class T, class Hash, class KeyEqual>
pair<typename hashtable<T, Hash, KeyEqual>::iterator, bool>
hashtable<T, Hash, KeyEqual>::insert_node_unique(node_ptr np) {
  const auto n = hash(value_traits::get_key(np->value));
  auto cur = buckets_[n];
  if (cur == nullptr) { // bucket 为空，直接插
    buckets_[n] = np;
    ++size_;
    return yastl::make_pair(iterator(np, this), true);
  }
  for (; cur; cur = cur->next) { // 遍历，已经存在的话返回失败
    if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(np->value))) {
      return yastl::make_pair(iterator(cur, this), false);
    }
  }
  np->next = buckets_[n]; // 插入头部 返回成功
  buckets_[n] = np;
  ++size_;
  return yastl::make_pair(iterator(np, this), true);
}

// replace_bucket 函数，把现有的 bucket size 调整为 bucket_count
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::replace_bucket(size_type bucket_count) {
  bucket_type bucket(bucket_count); // 新建一个 bucket_count 的 vector
  if (size_ != 0) { // 需要调整，若为空则直接换节省效率
    for (size_type i = 0; i < bucket_size_; ++i) { // 对于每一个 bucket
      for (auto first = buckets_[i]; first; first = first->next) { // 遍历每一条 old hashnode
        auto tmp = create_node(first->value); // 复制 old hashnode
        const auto n = hash(value_traits::get_key(first->value), bucket_count); // 重新计算在新的 bucket 中的索引
        auto f = bucket[n]; // 新 bucket 的首个 hashnode
        bool is_inserted = false;
        for (auto cur = f; cur; cur = cur->next) { // 遍历 new bucket 的 hashnode
          if (is_equal(value_traits::get_key(cur->value), value_traits::get_key(first->value))) {
            tmp->next = cur->next; // 如果 new bucket 的某个 node 值和 old 的键值相等
            cur->next = tmp; // 把 tmp 节点插入到 cur 后面，保证相同值在一块
            is_inserted = true;
            break;
          }
        }
        if (!is_inserted) {
          tmp->next = f; // 把 tmp 插入到 new bucket 的头部
          bucket[n] = tmp;
        }
      }
    }
  }
  buckets_.swap(bucket); // 交换，出了函数会自动析构 bucket
  bucket_size_ = buckets_.size();
}

// erase_bucket 函数
// 在第 n 个 bucket 内，删除 [first, last) 的节点
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::erase_bucket(size_type n, node_ptr first, node_ptr last) {
  auto cur = buckets_[n];
  if (cur == first) { // 清空
    erase_bucket(n, last);
  } else {
    node_ptr next = cur->next;
    for (; next != first; cur = next, next = cur->next) {} // 遍历直到 cur->next == first
    while (next != last) {
      cur->next = next->next; // 依次删除
      destroy_node(next);
      next = cur->next;
      --size_;
    }
  }
}

// erase_bucket 函数
// 在第 n 个 bucket 内，删除 [buckets_[n], last) 的节点
template <class T, class Hash, class KeyEqual>
void hashtable<T, Hash, KeyEqual>::erase_bucket(size_type n, node_ptr last) {
  auto cur = buckets_[n];
  while (cur != last) {
    auto next = cur->next;
    destroy_node(cur);
    cur = next;
    --size_;
  }
  buckets_[n] = last;
}

// equal_to 函数
// 这函数写的有问题 跑不了
template <class T, class Hash, class KeyEqual>
bool hashtable<T, Hash, KeyEqual>::equal_to_multi(const hashtable& other) {
  if (size_ != other.size_) {
    return false;
  }
  for (auto f = begin(), l = end(); f != l;) {
    auto p1 = equal_range_multi(value_traits::get_key(*f)); // 找到相等值的区间
    auto p2 = other.equal_range_multi(value_traits::get_key(*f));
    if (yastl::distance(p1.last, p1.last) != yastl::distance(p2.first, p2.last) ||
        !yastl::is_permutation(p1.first, p2.last, p2.first, p2.last)) { // 比较个数 以及 数值相等  这里是不写错了
      return false;
    }
    f = p1.last;
  }
  return true;
}

// 不允许重复的哈希表中 判断相等
template <class T, class Hash, class KeyEqual>
bool hashtable<T, Hash, KeyEqual>::equal_to_unique(const hashtable& other) {
  if (size_ != other.size_) { // 大小相等，否则 是 other 的子集下面也会返回 true
    return false;
  }
  for (auto f = begin(), l = end(); f != l; ++f) {
    auto res = other.find(value_traits::get_key(*f));
    if (res.node == nullptr || *res != *f) { // 只要某个元素在表里都有就可以
      return false;
    }
  }
  return true;
}

// 重载 yastl 的 swap
template <class T, class Hash, class KeyEqual>
void swap(hashtable<T, Hash, KeyEqual>& lhs, hashtable<T, Hash, KeyEqual>& rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace yastl
#endif // _INCLUDE_HASHTABLE_H_

