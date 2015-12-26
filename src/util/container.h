#ifndef UTIL_CONTAINER_H
#define UTIL_CONTAINER_H

#include "util/fixed_types.h"
#include <functional>

namespace Util {

class Container
{
 public:
  // Custom functions run prior to deallocation (e.g. to securely wipe)
  typedef std::function<void(Byte *, U64)> ScrubType;

  // Container methods
  Container(U64 size = 0, ScrubType scrubber = ScrubType());
  Container(const Container &) = delete;
  Container(Container &&) = default;
  Container &operator=(const Container &) = delete;
  Container &operator=(Container &&) = default;
  Byte *data() const;
  U64 size() const;
  ~Container();

 private:
  Byte *data_;
  U64 size_;
  ScrubType scrubber_;
};


// The default "scrubber" does nothing to the data
auto scrub_null = [] (Byte *, U64) {};

// A scrubber which overwrites all data with zeros
auto scrub_zeros = [] (Byte *data, U64 size)
{
  U64 words = size / 8;
  U64 bytes = size - (8 * words);
  U64 i;

  volatile U64 *vword = (U64 *)data;
  for (i = 0; i < words; i++) {
    vword[i] = 0L;
  }
  volatile Byte *vbyte = &data[size - bytes];
  for (i = 0; i < bytes; i++) {
    vbyte[i] = 0;
  }
};

} // namespace Util


#endif // UTIL_CONTAINER_H

