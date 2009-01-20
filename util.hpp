/*
 * util.hpp
 *
 *  Created on: Dec 15, 2008
 *      Author: mikosz
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <limits>

template <class T1, class T2>
T1 min(T1 lhs, T2 rhs)
{
    return lhs < rhs ? lhs : rhs;
}

template <class T1, class T2>
T1 max(T1 lhs, T2 rhs)
{
    return rhs < lhs ? lhs : rhs;
}

template <class T1, class T2>
T1 absDiff(T1 lhs, T2 rhs)
{
    return lhs < rhs ? rhs - lhs : lhs - rhs;
}

const size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();

size_t decrement(size_t rhs)
{
    return rhs ? rhs - 1 : 0;
}

#endif /* UTIL_HPP_ */
