#ifndef UTIL_COMPARE_H
#define UTIL_COMPARE_H

#include "util/blob.h"
#include "util/fixed_types.h"
#include <functional>
#include <cstring>

namespace Util {

// Standard comparison of two Blobs (may terminate early if different)
//
// Returns true if not equal, false if equal
auto compare_memcmp = [] (const Blob &_a, const Blob &_b) -> bool
{
  if (_a.size() != _b.size()) {
    return true;
  }
  return memcmp((const void *)_a.data(), (const void *)_b.data(), _a.size());
};

// 'Constant' time comparison (Theta(n)) for two Blobs of the same size, i.e.
// the comparison time is only dependent on the size of the inputs and not
// on their equality. If the Blobs are of different sizes the comparison
// immediately returns not equal.
//
// Returns true if not equal, false if equal
auto compare_constant = [] (const Blob &_a, const Blob &_b) -> bool
{
  U64 size_bytes = _a.size();
  if (size_bytes != _b.size()) {
    return true;
  }

  const Byte *ap = _a.data();
  const Byte *bp = _b.data();

  // Compare with full 64-bit words
  const U64 *awp = (const U64 *) ap;
  const U64 *bwp = (const U64 *) bp;
  U64 size_words = size_bytes >> 3;
  U64 result = 0;
  for (U64 i = 0; i < size_words; i++) {
    result |= *awp++ ^ *bwp++;
  }

  // Compare remaining bytes
  U64 offset = size_words * sizeof(U64);
  ap += offset;
  bp += offset;
  for (U64 i = offset; i < size_bytes; i++) {
    result |= (U64) *ap++ ^ (U64) *bp++;
  }
  return result;
};

} // namespace Util


#endif // UTIL_COMPARE_H

