#include "util/blob.h"
#include "util/compare.h"
#include <cstring>  // XXX del

using namespace Util;
using std::string;
using std::unique_ptr;
using std::make_shared;

Blob::Blob(U64 _size, ScrubType _scrubType, Blob::CompareType _compareType)
  : container_(make_shared<Container>(_size, scrubberForType(_scrubType))), scrubType_(_scrubType),
  compareType_(_compareType), comparator_(comparatorForType(_compareType)),
  data_(container_->data()), size_(container_->size())
{
  // empty
}

Blob::Blob(const Byte *_stream, U64 _size, ScrubType _scrubType, Blob::CompareType _compareType)
  : container_(make_shared<Container>(_size, scrubberForType(_scrubType))),
  scrubType_(_scrubType), compareType_(_compareType), comparator_(comparatorForType(_compareType)),
  data_(container_->data()), size_(container_->size())
{
  memcpy((void *)container_->data(), (const void *)_stream, container_->size());
}

Blob::Blob(const char *_stream, U64 _size, ScrubType _scrubType, Blob::CompareType _compareType)
  : Blob((const Byte *)_stream, _size, _scrubType, _compareType)
{
  // empty
}

Blob::Blob(const Blob &_other, U64 _size, U64 _offset)
  : container_(_other.container_), scrubType_(_other.scrubType_),
  comparator_(_other.comparator_), data_(nullptr), size_(0)
{
  // Sanitize inputs to prevent integer and buffer overflow opportunities
  // (only possible when copying Blobs; no way to know with Byte* buffers).
  if (_offset > _other.size_) {
    _offset = _other.size_;
  }
  if (_size > (_other.size_ - _offset)) {
    _size = _other.size_ - _offset;
  }
  data_ = &(_other.data_[_offset]);
  size_ = _size;
}

Blob::Blob(const Byte *_data, U64 _size, Decoder _decoder)
{
  unique_ptr<Blob> b = _decoder(_data, _size);
  *this = std::move(*b);
}

Blob::Blob(const string &_data)
  : Blob((const Byte *)_data.data(), _data.size())
{
  // empty
}

Blob::Blob(const string &_data, Decoder _decoder)
  : Blob((const Byte *)_data.data(), _data.size(), _decoder)
{
  // empty
}

Blob::Blob(std::initializer_list<Blob> _blobs, ScrubType _scrubType, Blob::CompareType _compareType)
  : container_(), scrubType_(_scrubType), compareType_(_compareType), comparator_(comparatorForType(_compareType)),
  data_(nullptr), size_(0)
{
  // Determine the aggregate size of all Blobs
  for (auto blob : _blobs) {
    size_ += blob.size();
  }

  // Combine (copy) the Blobs into one
  container_ = make_shared<Container>(size_, scrubberForType(scrubType_));
  Byte *mdata = container_->data();
  data_ = mdata;
  U64 offset = 0;
  for (auto blob : _blobs) {
    memcpy((void *)&(mdata[offset]), blob.data(), blob.size());
    offset += blob.size();
  }
}

bool Blob::operator==(const Blob &_other) const
{
  return !operator!=(_other);
}

bool Blob::operator!=(const Blob &_other) const
{
  // Two Blobs are equal when their data is equal, even if from
  // separate data streams
  return comparator_(*this, _other);
}

const Byte &Blob::operator[](U64 _index) const
{
  return data_[_index];
}

Blob::Comparison Blob::compare(const Blob &_other, CompareType _compareType)
{
  Comparator cmp = comparatorForType(_compareType);
  if (cmp(*this, _other)) {
    return Comparison::NE;
  }
  else {
    return Comparison::EQ;
  }
}

void Blob::dataIs(const Byte *_stream, U64 _size, ScrubType _scrubType, Blob::CompareType _compareType)
{
  container_ = make_shared<Container>(_size, scrubberForType(_scrubType));
  scrubType_ = _scrubType;
  compareType_ = _compareType;
  comparator_ = comparatorForType(_compareType);
  data_ = container_->data();
  size_ = container_->size();
  memcpy((void *)container_->data(), (const void *)_stream, container_->size());
}

void Blob::dataIs(const char *_stream, U64 _size, ScrubType _scrubType, Blob::CompareType _compareType)
{
  dataIs((const Byte *)_stream, _size, _scrubType, _compareType);
}

void Blob::dataIsNull()
{
  container_ = make_shared<Container>(0, scrubberForType(ScrubType::NONE));
  scrubType_ = ScrubType::NONE;
  comparator_ = comparatorForType(Blob::CompareType::DEFAULT);
  data_ = nullptr;
  size_ = 0;
}

const Byte *Blob::data() const
{
  return data_;
}

unique_ptr<string> Blob::data(Encoder _encoder) const
{
  return _encoder(data_, size_);
}

U64 Blob::size() const
{
  return size_;
}

Blob::ScrubType Blob::scrubType() const
{
  return scrubType_;
}

Blob::CompareType Blob::compareType() const
{
  return compareType_;
}

Container::ScrubType Blob::scrubberForType(ScrubType _scrubType)
{
  if (_scrubType == Blob::ScrubType::ZEROS) {
    // Write zeros to the data before deallocation
    return scrub_zeros;
  }
  else {
    // An empty lambda for ScrubType::NONE
    return scrub_null;
  }
}

Blob::Comparator Blob::comparatorForType(Blob::CompareType _compareType)
{
  if (_compareType == Blob::CompareType::DEFAULT) {
    return compare_memcmp;
  }
  else {
    return compare_constant;
  }
}


// MutableBlob

MutableBlob::MutableBlob(U64 _size, ScrubType _scrubType, Blob::CompareType _compareType)
  : Blob(_size, _scrubType, _compareType)
{
  // empty
}

MutableBlob::MutableBlob(const Byte *_stream, U64 _size, ScrubType _scrubType, Blob::CompareType _compareType)
  : Blob(_stream, _size, _scrubType, _compareType)
{
  // empty
}

MutableBlob::MutableBlob(const Blob &_other, ScrubType _scrubType, Blob::CompareType _compareType)
  : MutableBlob(_other.data(), _other.size(), _scrubType, _compareType)
{
  // empty
}

Byte &MutableBlob::operator[](U64 _index)
{
  return container_->data()[_index];
}

Byte *MutableBlob::data()
{
  return container_->data();
}

