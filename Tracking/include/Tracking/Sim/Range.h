// This file is part of the Acts project.
//
// Copyright (C) 2019 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iterator>
#include <utility>

namespace acts_examples {

/// A wrapper around a pair of iterators to simplify range-based loops.
///
/// Some standard library algorithms return pairs of iterators to identify
/// a sub-range. This wrapper simplifies the iteration and should be used as
/// follows:
///
///     for (auto indx : makeRange(std::equal_range(...)) {
///         ...
///     }
///
template <typename Iterator>
class Range {
 public:
  Range(Iterator b, Iterator e) : m_begin_(b), m_end_(e) {}
  Range(Range&&) = default;
  Range(const Range&) = default;
  ~Range() = default;
  Range& operator=(Range&&) = default;
  Range& operator=(const Range&) = default;

  Iterator begin() const { return m_begin_; }
  Iterator end() const { return m_end_; }
  bool empty() const { return m_begin_ == m_end_; }
  std::size_t size() const { return std::distance(m_begin_, m_end_); }

 private:
  Iterator m_begin_;
  Iterator m_end_;
};

template <typename Iterator>
Range<Iterator> makeRange(Iterator begin, Iterator end) {
  return Range<Iterator>(begin, end);
}

template <typename Iterator>
Range<Iterator> makeRange(std::pair<Iterator, Iterator> range) {
  return Range<Iterator>(range.first, range.second);
}

}  // namespace ActsExamples
