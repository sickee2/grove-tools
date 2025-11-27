/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 */

#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <gr/config.hh>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace gr {
using void_ptr = void *;
namespace utils {

/**
 * @brief get original type from pointer type
 * @param Tp
 */
template <typename Tp>
using pointer_to_type = std::remove_const_t<std::remove_pointer_t<Tp>>;

GR_CONSTEXPR_OR_INLINE const static size_t nopos = size_t(-1);

/**
 * @brief Custom smart pointer with explicit memory management
 * @tparam T Type of the managed object
 *
 * A lightweight smart pointer that uses new/delete for memory allocation
 * and requires explicit destructor calls. Designed for POD types and simple
 * objects without complex inheritance hierarchies.
 *
 * Features:
 * - Move-only semantics (copy operations are explicitly deleted)
 * - Factory method pattern for object creation
 * - Explicit memory management with new/delete
 * - Placement new for object construction
 *
 * @warning Not suitable for objects with virtual functions or complex
 * inheritance
 * @warning Uses new/delete for memory management
 */
template <typename T> class cptr {
  /**
   * @brief Copy assignment is explicitly deleted
   * @param p Other cptr (not used)
   */
  cptr &operator=(cptr &p) =
      GR_DELETE_FUNC("please use std::move assignment instead of copy");

  /**
   * @brief Copy constructor is explicitly deleted
   * @param p Other cptr (not used)
   */
  cptr(cptr &p) =
      GR_DELETE_FUNC("please use std::move construct instead of copy");

public:
  /**
   * @brief Default constructor - creates empty pointer
   */
  constexpr cptr() noexcept : m_ptr(nullptr) {}

  /**
   * @brief Destructor - explicitly calls destructor and frees memory
   */
  ~cptr() {
    delete m_ptr;
    m_ptr = nullptr;
  }

  /**
   * @brief Dereference operator
   * @return Reference to the managed object
   * @throws Undefined behavior if pointer is null
   */
  inline T &operator*() const { return *m_ptr; }

  /**
   * @brief Member access operator
   * @return Pointer to the managed object
   */
  T *operator->() noexcept { return m_ptr; }
  const T *operator->() const noexcept { return m_ptr; }

  /**
   * @brief Get the raw pointer
   * @return Raw pointer to the managed object
   */
  T *get() const noexcept { return m_ptr; }

  /**
   * @brief Swap contents with another cptr
   * @param p Other cptr to swap with
   */
  void swap(cptr &p) noexcept {
    T *tmp = m_ptr;
    m_ptr = p.m_ptr;
    p.m_ptr = tmp;
  }

  /**
   * @brief Reset to manage a new object
   * @tparam Args Types of constructor arguments
   * @param args Arguments to forward to object constructor
   * @note Releases current object before creating new one
   */
  template <typename... Args> void reset(Args... args) {
    delete m_ptr;
    m_ptr = new T(args...);
  }

  /**
   * @brief Move assignment operator
   * @param p Source cptr to move from
   * @return Reference to this cptr
   * @note The source cptr will be left in empty state
   */
  cptr &operator=(cptr &&p) noexcept {
    if (this == &p)
      return *this;
    delete m_ptr;
    m_ptr = p.m_ptr;
    p.m_ptr = nullptr;
    return *this;
  }

  /**
   * @brief Check if pointer is null
   * @return true if pointer is null, false otherwise
   */
  bool is_null() const noexcept { return m_ptr == nullptr; }

  /**
   * @brief Boolean conversion operator
   * @return true if pointer is not null, false otherwise
   */
  operator bool() const noexcept { return m_ptr != nullptr; }

  /**
   * @brief Move constructor
   * @param p Source cptr to move from
   * @note The source cptr will be left in empty state
   */
  cptr(cptr &&p) noexcept : m_ptr(p.m_ptr) { p.m_ptr = nullptr; }

  /**
   * @brief Create a deep copy of the managed object
   * @return cptr<T> containing a copy of the current object
   * @throws std::runtime_error if memory allocation fails
   * @note Requires T to be copy constructible
   */
  cptr<T> clone() const { return m_ptr ? cptr<T>::make(*m_ptr) : cptr<T>(); }

  /**
   * @brief Factory method to create a cptr with constructed object
   * @tparam Args Types of constructor arguments
   * @param args Arguments to forward to object constructor
   * @return cptr managing the newly created object
   *
   * Usage:
   * @code
   * auto ptr = cptr<MyClass>::make(arg1, arg2);
   * @endcode
   */
  template <typename... Args> static cptr make(Args... args) {
    cptr tmp;
    tmp.m_ptr = new T(args...);
    return tmp;
  }

private:
  T *m_ptr;
  static_assert(std::is_object_v<T> && !std::is_array_v<T>, 
                "cptr can only manage single objects, not arrays");
  static_assert(std::is_destructible_v<T>, 
                "cptr requires destructible types");
};

/**
 * @brief Helper function to create cptr objects
 * @tparam T Type of object to manage
 * @tparam Args Types of constructor arguments
 * @param args Arguments to forward to object constructor
 * @return cptr<T> managing the newly created object
 */
template <typename T, typename... Args> auto make_cptr(Args... args) {
  return cptr<T>::make(args...);
}

template <typename T>
constexpr bool _is_pod_type =
    std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> &&
    std::is_trivially_destructible_v<T>;
/**
 * @brief Simple buffer container for POD (Plain Old Data) types
 * @tparam T Element type (must satisfy PodType concept)
 *
 * A lightweight buffer container that provides RAII management for
 * dynamically allocated arrays of POD types. Uses malloc/free for
 * memory management and provides basic container operations.
 *
 * Key Design Considerations:
 * - Uses malloc/free instead of new/delete[] to avoid constructor/destructor
 * calls
 * - Relies on std::memcpy for data copying operations
 * - Supports realloc for efficient memory growth without data copying overhead
 *
 * Features:
 * - RAII memory management with automatic cleanup
 * - Move semantics support (transfer ownership without copying)
 * - Basic container interface (begin, end, size, operator[])
 * - Memory reallocation support with growth-only policy
 * - Zero-initialization capability
 * - Memory ownership transfer via detach()
 *
 * @warning STRICTLY for POD types only (trivially copyable, standard layout,
 * and trivially destructible)
 * @warning NOT suitable for types with non-trivial constructors/destructors
 * @warning Uses malloc/free instead of new/delete[] to avoid
 * constructor/destructor calls
 * @warning realloc() may move memory to a new location - invalidating existing
 * pointers/references
 */
template <typename T> class cbuf {
  static_assert(_is_pod_type<T>, "cbuf requires POD type (trivially copyable, standard layout, trivially destructible)");
  cbuf(const cbuf &v) = GR_DELETE_FUNC("please use std::move to assignment");
  cbuf &
  operator=(cbuf &v) = GR_DELETE_FUNC("please use std::move to assignment");

public:
  /**
   * @brief Create a buffer with specified capacity
   * @param n Number of elements to allocate
   * @return cbuf instance with allocated memory
   * @throws std::runtime_error if memory allocation fails
   */
  static cbuf create(size_t n) {
    if (n == 0)
      return cbuf();
    T *tmp = (T *)std::malloc(sizeof(T) * n);
    if (!tmp) {
      throw std::runtime_error("gr::cbuf::create => memory allocation failed");
    }
    return cbuf(tmp, tmp + n);
  }

  /**
   * @brief Default constructor - creates empty buffer
   */
  cbuf() noexcept : _beg(nullptr), _end(nullptr) {}

  /**
   * @brief Destructor - frees allocated memory
   */
  ~cbuf() noexcept {
    if (_beg) {
      std::free(_beg);
      _beg = _end = nullptr;
    }
  }

  /**
   * @brief Move constructor - transfers ownership
   * @param v Source buffer to move from
   * @note Source buffer will be left in empty state
   */
  cbuf(cbuf &&v) noexcept : _beg(v._beg), _end(v._end) { v._beg = v._end = nullptr; }

  /**
   * @brief Force assingment operator as move assignment operator
   * @param v Source buffer to move from
   * @return Reference to this buffer
   * @note Source buffer will be left in empty state
   */
  cbuf &operator=(cbuf &&v) {
    if (this == &v) // Check is self
      return *this;
    if (_beg)
      std::free(_beg);
    _beg = v._beg;
    _end = v._end;
    v._beg = v._end = nullptr;
    return *this;
  }

  /**
   * @brief Boolean conversion operator
   * @return true if buffer is not empty, false otherwise
   */
  operator bool() const noexcept { return _beg ? true : false; }

  /**
   * @brief deep copy the buffer
   * @return new cbuf object
   */
  cbuf clone() const {
    if (!_beg)
      return cbuf();
    size_t sz = this->size();
    T *beg_ = (T *)std::malloc(sizeof(T) * sz);
    if (!beg_) {
      throw std::runtime_error("gr::cbuf::clone => memory allocation failed");
    }
    T *end_ = beg_ + sz;
    std::memcpy(beg_, _beg, sz * sizeof(T));
    return cbuf(beg_, end_);
  }

  /**
   * @brief Reallocate buffer to new size (grow only)
   * @param n New number of elements
   * @throws std::runtime_error if reallocation fails
   * @note Only grows the buffer, does not shrink
   */
  void realloc(size_t n) {
    size_t sz = _end - _beg;
    if (n <= sz) {
      return;
    }

    T *tmp = (T *)std::realloc(_beg, sizeof(T) * n);
    if (tmp) {
      _beg = tmp;
      _end = _beg + n;
    } else {
      throw std::runtime_error("gr::cbuf::realloc => failed!");
    }
  }

  /**
   * @brief Element access operator
   * @param index Element index
   * @return Reference to element at specified index
   * @note No bounds checking performed
   */
  T &operator[](size_t index) noexcept { return *(_beg + index); }
  const T &operator[](size_t index) const noexcept { return *(_beg + index); }

  /**
   * @brief Iterator to beginning of buffer
   * @return Pointer to first element
   */
  T *begin() const noexcept { return _beg; }

  /**
   * @brief Iterator to end of buffer
   * @return Pointer to one past last element
   */
  T *end() const noexcept { return _end; }

  /**
   * @brief Get number of elements in buffer
   * @return Number of elements
   */
  GR_CONSTEXPR_OR_INLINE size_t size() const noexcept { return _end - _beg; }

  /**
   * @brief Get total bytes occupied by buffer
   * @return Size in bytes
   */
  GR_CONSTEXPR_OR_INLINE size_t bytes() const noexcept {
    return sizeof(T) * (_end - _beg);
  }

  /**
   * @brief Fill buffer with zeros
   */
  void fillzero() noexcept { std::memset((void *)(_beg), 0, bytes()); };

  /**
   * @brief Explicitly release the buffer memory
   * @note After calling this, the buffer becomes empty
   * @warning The memory is freed and all existing pointers/references become
   * invalid
   */
  void release() noexcept {
    if (_beg) {
      std::free(_beg);
      _beg = _end = nullptr;
    }
  }

  /**
   * @brief Swap contents with another cbuf
   * @param other Other cbuf to swap with
   * @note Both buffers exchange ownership of their memory
   */
  void swap(cbuf &other) noexcept {
    T *tmp_beg = _beg;
    T *tmp_end = _end;

    _beg = other._beg;
    _end = other._end;

    other._beg = tmp_beg;
    other._end = tmp_end;
  }

  /**
   * @brief Low-level memory attachment from existing pointer
   * @param p Pointer to existing memory block
   * @param n Number of elements in the memory block
   * @return cbuf instance that manages the attached memory
   * @note This is a low-level operation that takes ownership of existing memory
   * @warning The memory must have been allocated with malloc/calloc/realloc
   * @warning After attachment, the cbuf will manage the memory and free it on
   * destruction
   * @warning The pointer must not be null and n must be > 0
   * @warning Do not use with memory allocated by new/delete
   */
  static cbuf attach(T *p, size_t n) {
    if (!p || n == 0) {
      throw std::invalid_argument("gr::cbuf::attach => invalid pointer or size");
    }
    return cbuf{p, p + n};
  }

  /**
   * @brief Detach and transfer ownership of internal buffer
   * @return Pair containing pointer to buffer and element count
   * @note After calling this
   *       - the cbuf object becomes empty,
   *       - the memory should manage manually
   */
  [[nodiscard]] std::pair<T *, size_t> detach() noexcept {
    std::pair<T *, size_t> ret = {_beg, size()};
    _beg = _end = nullptr;
    return ret;
  }

  /**
   * @brief Convert buffer to different POD type (low-level memory
   * reinterpretation)
   * @tparam T2 Target POD type to for conversion
   * @return cbuf<T2> with reinterpreted
   * @note This is a low-level operation that reinterprets the existing memory
   * as a different type
   * @note Memory ownership is transfered - the original buffer becomes empty
   * @note No memory copying occurs - this is a zero-cost type reinterpretation
   * @warning The caller must ensure type compatibility and proper alignment
   * @warning Buffer size is measured in element, not bytes - size may change
   * with different types
   * @warning Use only with POD types that have compatible memory layout
   */
  template <typename T2> cbuf<T2> convert_as() noexcept {
    auto ret = cbuf<T2>();
    ret.swap(*((cbuf<T2> *)(this)));
    return ret;
  }

private:
  T *_beg; ///< Pointer to beginning of buffer
  T *_end; ///< Pointer to one past end of buffer

  /**
   * @brief Private constructor for internal use
   * @param ibeg Pointer to buffer start
   * @param iend Pointer to buffer end
   */
  cbuf(T *ibeg, T *iend) : _beg(ibeg), _end(iend) {}
};

} // namespace utils

template <typename T, typename... Args> auto make_cptr(Args... args) {
  return utils::cptr<T>::make(args...);
}

/**
 * @brief make simple buffer
 * @note Do not store class objects in this buffer
 */
template <typename T> auto make_cbuf(size_t n) {
  return utils::cbuf<T>::create(n);
}

/**
 * @brief Create buffer from initializer list
 * @tparam T Element type (must satisfy PodType concept)
 * @param list Initializer list with values
 * @return cbuf<T> containing copied values
 * @note Only for POD types
 */
template <typename T> auto make_cbuf(std::initializer_list<T> list) {
  auto buf = utils::cbuf<T>::create(list.size());
  if (buf) {
    std::memcpy(buf.begin(), list.begin(), list.size() * sizeof(T));
  }
  return buf;
}

} // namespace gr
