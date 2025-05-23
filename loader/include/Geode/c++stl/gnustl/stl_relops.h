// std::rel_ops implementation -*- C++ -*-

// Copyright (C) 2001-2025 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the, 2009 Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

/** @file bits/stl_relops.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{utility}
 *
 *  This file is only included by `<utility>`, which is required by the
 *  standard to define namespace `rel_ops` and its contents.
 */

#ifndef _STL_RELOPS_H
#define _STL_RELOPS_H 1

namespace geode::stl
{

  namespace rel_ops
  {
    /** @namespace std::rel_ops
     *  @brief  The generated relational operators are sequestered here.
     *
     *  Libstdc++ headers must not use the contents of `rel_ops`.
     *  User code should also avoid them, because unconstrained function
     *  templates are too greedy and can easily cause ambiguities.
     *
     *  C++20 default comparisons are a better solution.
     */

    /**
     *  @brief Defines @c != for arbitrary types, in terms of @c ==.
     *  @param  __x  A thing.
     *  @param  __y  Another thing.
     *  @return   __x != __y
     *
     *  This function uses @c == to determine its result.
     */
    template <class _Tp>
      inline bool
      operator!=(const _Tp& __x, const _Tp& __y)
      { return !(__x == __y); }

    /**
     *  @brief Defines @c > for arbitrary types, in terms of @c <.
     *  @param  __x  A thing.
     *  @param  __y  Another thing.
     *  @return   __x > __y
     *
     *  This function uses @c < to determine its result.
     */
    template <class _Tp>
      inline bool
      operator>(const _Tp& __x, const _Tp& __y)
      { return __y < __x; }

    /**
     *  @brief Defines @c <= for arbitrary types, in terms of @c <.
     *  @param  __x  A thing.
     *  @param  __y  Another thing.
     *  @return   __x <= __y
     *
     *  This function uses @c < to determine its result.
     */
    template <class _Tp>
      inline bool
      operator<=(const _Tp& __x, const _Tp& __y)
      { return !(__y < __x); }

    /**
     *  @brief Defines @c >= for arbitrary types, in terms of @c <.
     *  @param  __x  A thing.
     *  @param  __y  Another thing.
     *  @return   __x >= __y
     *
     *  This function uses @c < to determine its result.
     */
    template <class _Tp>
      inline bool
      operator>=(const _Tp& __x, const _Tp& __y)
      { return !(__x < __y); }
  } // namespace rel_ops

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

#endif /* _STL_RELOPS_H */
