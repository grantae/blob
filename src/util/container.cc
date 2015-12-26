#include "util/container.h"

using namespace Util;

Container::Container(U64 _size, ScrubType _scrubber)
  : data_(new Byte[_size]), size_(_size), scrubber_(_scrubber)
{
  // empty
}

Container::~Container()
{
  // Run the scrubber, whatever it is
  scrubber_(data_, size_);
  delete[] data_;
}

Byte *Container::data() const
{
  return data_;
}

U64 Container::size() const
{
  return size_;
}

