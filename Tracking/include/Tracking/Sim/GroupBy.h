// This file is part of the Acts project.
//
// Copyright (C) 2020 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <iterator>
#include <utility>

#include "Tracking/Sim/Range.h"

namespace acts_examples {

/// Proxy for iterating over groups of elements within a container.
///
/// @note Each group will contain at least one element.
///
/// Consecutive elements with the same key (as defined by the KeyGetter) are
/// placed in one group. The proxy should always be used as part of a
/// range-based for loop. In combination with structured bindings to reduce the
/// boilerplate, the group iteration can be written as
///
///     for (auto&& [key, elements] : GroupBy<...>(...)) {
///         // do something with just the key
///         ...
///
///         // iterate over the group elements
///         for (const auto& element : elements) {
///             ...
///         }
///     }
///
template <typename Iterator, typename KeyGetter>
class GroupBy {
 public:
  /// The key type that identifies elements within a group.
  using Key = std::decay_t<decltype(KeyGetter()(*Iterator()))>;
  /// A Group is an iterator range with the associated key.
  using Group = std::pair<Key, Range<Iterator>>;
  /// Iterator type representing the end of the groups.
  ///
  /// The end iterator will not be dereferenced in C++17 range-based loops. It
  /// can thus be a simpler type without the overhead of the full group iterator
  /// below.
  using GroupEndIterator = Iterator;
  /// Iterator type representing a group of elements.
  class GroupIterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Group;
    using difference_type = std::ptrdiff_t;
    using pointer = Group*;
    using reference = Group&;

    constexpr GroupIterator(const GroupBy& groupBy, Iterator groupBegin)
        : m_group_by_(groupBy),
          m_group_begin_(groupBegin),
          m_group_end_(groupBy.findEndOfGroup(groupBegin)) {}
    /// Pre-increment operator to advance to the next group.
    constexpr GroupIterator& operator++() {
      // make the current end the new group beginning
      std::swap(m_group_begin_, m_group_end_);
      // find the end of the next group starting from the new beginning
      m_group_end_ = m_group_by_.findEndOfGroup(m_group_begin_);
      return *this;
    }
    /// Post-increment operator to advance to the next group.
    constexpr GroupIterator operator++(int) {
      GroupIterator retval = *this;
      ++(*this);
      return retval;
    }
    /// Derefence operator that returns the pointed-to group of elements.
    constexpr Group operator*() const {
      const Key key = (m_group_begin_ != m_group_end_)
                          ? m_group_by_.m_key_getter_(*m_group_begin_)
                          : Key();
      return {key, makeRange(m_group_begin_, m_group_end_)};
    }

   private:
    const GroupBy& m_group_by_;
    Iterator m_group_begin_;
    Iterator m_group_end_;

    friend constexpr bool operator==(const GroupIterator& lhs,
                                     const GroupEndIterator& rhs) {
      return lhs.m_group_begin_ == rhs;
    }
    friend constexpr bool operator!=(const GroupIterator& lhs,
                                     const GroupEndIterator& rhs) {
      return not(lhs == rhs);
    }
  };

  /// Construct the group-by proxy for an iterator range.
  constexpr GroupBy(Iterator begin, Iterator end,
                    KeyGetter keyGetter = KeyGetter())
      : m_begin_(begin), m_end_(end), m_key_getter_(std::move(keyGetter)) {}
  constexpr GroupIterator begin() const {
    return GroupIterator(*this, m_begin_);
  }
  constexpr GroupEndIterator end() const { return m_end_; }
  constexpr bool empty() const { return m_begin_ == m_end_; }

 private:
  Iterator m_begin_;
  Iterator m_end_;
  KeyGetter m_key_getter_;

  /// Find the end of the group that starts at the given position.
  ///
  /// This uses a linear search from the start position and thus has linear
  /// complexity in the group size. It does not assume any ordering of the
  /// underlying container and is a cache-friendly access pattern.
  constexpr Iterator findEndOfGroup(Iterator start) const {
    // check for end so we can safely dereference the start iterator.
    if (start == m_end_) {
      return start;
    }
    // search the first element that does not share a key with the start.
    return std::find_if_not(std::next(start), m_end_,
                            [this, start](const auto& x_) {
                              return m_key_getter_(x_) == m_key_getter_(*start);
                            });
  }
};

/// Construct the group-by proxy for a container.
template <typename Container, typename KeyGetter>
auto makeGroupBy(const Container& container, KeyGetter keyGetter)
    -> GroupBy<decltype(std::begin(container)), KeyGetter> {
  return {std::begin(container), std::end(container), std::move(keyGetter)};
}

}  // namespace ActsExamples
