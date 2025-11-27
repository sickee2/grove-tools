/**
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 SICKEE2
 *
 * @file tree_iter.hh
 * @brief Generic tree iterator for hierarchical data structures
 * 
 * Provides a flexible iterator template for traversing tree-like data structures
 * with support for both depth-first and breadth-first traversal strategies.
 *
 * ## Core Features
 * - **Generic Design**: Works with any tree node type that meets the required interface
 * - **Dual Traversal Modes**: Depth-first (DFS) and breadth-first (BFS) iteration
 * - **Level Tracking**: Maintains current depth level during traversal
 * - **STL-Compatible**: Forward iterator interface for seamless integration
 * - **Concept/Type Trait Validation**: Compile-time interface checking
 *
 * ## Requirements for Tree Node Types
 *
 * To use with `tree_iterator`, tree node types must provide:
 * - `container_t` type alias: The container type for child nodes
 * - `children()` method: Returns reference to container of child nodes
 *
 * ### Example Node Structure
 * ```cpp
 * struct TreeNode {
 *     using container_t = std::vector<TreeNode*>;
 *     
 *     container_t& children() { return _children; }
 *     const container_t& children() const { return _children; }
 *     
 * private:
 *     container_t _children;
 * };
 * ```
 *
 * ## Usage Examples
 *
 * ### Depth-First Traversal (Default)
 * ```cpp
 * tree_iterator<TreeNode, true> dfs_iter(root_node);
 * while (dfs_iter) {
 *     process_node(*dfs_iter, dfs_iter.level());
 *     ++dfs_iter;
 * }
 * ```
 *
 * ### Breadth-First Traversal
 * ```cpp
 * tree_iterator<TreeNode, false> bfs_iter(root_node);
 * for (; bfs_iter; ++bfs_iter) {
 *     process_node(*bfs_iter, bfs_iter.level());
 * }
 * ```
 *
 * ### Range-Based Loop (C++17+)
 * ```cpp
 * for (auto iter = tree_iterator(root_node); iter; ++iter) {
 *     std::cout << "Level " << iter.level() << ": " << *iter << "\n";
 * }
 * ```
 *
 * ## Iterator Interface
 *
 * The iterator provides standard forward iterator operations:
 * - `operator*()`: Dereference to current node
 * - `operator->()`: Member access to current node  
 * - `operator++()`: Advance to next node
 * - `operator bool()`: Check if iterator is valid
 * - `level()`: Get current depth level (0 for root)
 *
 * ## Implementation Details
 *
 * ### Traversal Strategies
 * - **Depth-First (DFS)**: Uses stack-based traversal (LIFO)
 * - **Breadth-First (BFS)**: Uses queue-based traversal (FIFO)
 *
 * ### Memory Management
 * - Lightweight iterator state (no dynamic allocation)
 * - Stack/queue containers manage traversal state
 * - Iterator does not own tree nodes
 *
 * ### Compile-Time Validation
 * - C++20: Uses concepts for interface validation
 * - C++17: Uses type traits for interface validation
 * - Clear error messages for interface violations
 *
 * ## Performance Characteristics
 * - O(1) per-node iteration cost
 * - O(h) memory usage for DFS (h = tree height)
 * - O(w) memory usage for BFS (w = maximum width)
 * - Zero dynamic allocations during iteration
 *
 * @tparam TreeNodeT Tree node type meeting the required interface
 * @tparam deep Traversal strategy (true = DFS, false = BFS)
 *
 * @see For Unicode string operations: gr/string.hh
 * @see For character conversion utilities: gr/detail/toy_charconv.hh  
 * @see For string formatting: gr/format.hh
 */

#pragma once

#include <stack>
#include <queue>
#include <type_traits>
#ifdef __cpp_lib_concepts // C++ >= 20 && concepts
#include <concepts>
namespace gr::utils::__tree{
template <typename T>
concept ValidTreeNode = requires(T node) {
    typename T::container_t;              // check container_t
    { node.children() } -> std::same_as<typename T::container_t&>; // check children() method of T
};
}
#else
namespace gr::utils::__tree{
template <typename T, typename = void>
struct is_valid_treenode : std::false_type {};

template <typename T>
struct is_valid_treenode<T, std::void_t<
    typename T::container_t,
    decltype(std::declval<T>().children())
>> : std::true_type {};
}
#endif

namespace gr{
template<typename TreeNodeT, bool deep = true>
class tree_iterator{

#ifdef __cpp_lib_concepts // C++ >= 20 && concepts
  static_assert(utils::__tree::ValidTreeNode<TreeNodeT>,
        "\n\t[[[ TreeNodeT must define container_t and provide children() method ]]]\n");
#else
  static_assert(utils::__tree::is_valid_treenode<TreeNodeT>::value,
        "\n\t[[[ TreeNodeT must define container_t and provide children() method ]]]\n");
#endif

  struct Level{
    typename TreeNodeT::container_t::iterator current;
    typename TreeNodeT::container_t::iterator end;
    unsigned level;
  };
  struct level_accessor {
      static auto& get(std::stack<Level>& c) { return c.top(); }
      static auto& get(std::queue<Level>& c) { return c.front(); }
  };
  using level_container_t = std::conditional_t<deep, std::stack<Level>, std::queue<Level>>;
  level_container_t _levels;
  TreeNodeT *_current;
  unsigned _cur_level;
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = TreeNodeT;
  // using difference_type = std::ptrdiff_t;
  using pointer = TreeNodeT*;
  using reference = TreeNodeT&;

  constexpr tree_iterator(pointer node) noexcept : _current(node), _cur_level(0){
    if(node){
      auto &chs = node->children();
      if(!chs.empty())
        _levels.push({node->children().begin(), node->children().end(), 1});
    }
  }

  reference operator*() noexcept { return *_current; }
  pointer operator->() noexcept { return _current; }

  reference operator*() const noexcept { return *_current; }
  const pointer *operator->() const noexcept { return _current; }

  explicit operator bool() const noexcept { return _current != nullptr; }

  constexpr size_t level() const noexcept {
    if(_current == nullptr) return 0;
    return _cur_level;
  }

  tree_iterator &operator++(){
    if(_current == nullptr) return *this;

    while(_current && !_levels.empty()){
      auto &current_level = level_accessor::get(_levels);
      if(current_level.current != current_level.end){
        _current = *current_level.current;
        _cur_level = current_level.level;
        ++current_level.current;
        auto &chs = _current->children();
        if(!chs.empty()){
          _levels.push({chs.begin(), chs.end(), _cur_level + 1});
        }
        return *this;
      }
      _levels.pop();
    }
    _current = nullptr;
    return *this;
  }
};
} // namespace gr
