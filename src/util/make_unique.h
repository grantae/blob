#ifndef UTIL_MAKE_UNIQUE_H
#define UTIL_MAKE_UNIQUE_H

#include <utility>
#include <memory>

/* This is an implementation of 'make_unique' which was mistakenly
   omitted from the C++11 standard. If you can compile with C++14
   then this header is not needed. However, it can coexist with
   C++14 because it's in the 'Util' namespace.
*/

namespace Util {

template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // namespace Util

#endif // UTIL_MAKE_UNIQUE_H

