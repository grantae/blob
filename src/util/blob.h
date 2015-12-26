#ifndef UTIL_BLOB_H
#define UTIL_BLOB_H

#include "util/container.h"
#include "util/fixed_types.h"
#include <initializer_list>
#include <functional>
#include <string>
#include <memory>

namespace Util {

/*
   A "blob" is a collection of bytes which can contain any type of data.
   The Blob class acts as a container for and pointer to its underlying data.
   Blobs can be created as subsets of other Blobs without copying any portion
   of the underlying data. Blobs can be created and deleted without affecting
   other blobs, even when they share the same data.

   The primary motivation for the Blob class is to be able to work on several
   portions of a large piece of data without ever needing to copy the data
   and without needing to think about memory management. The following is a summary
   of the features and behavior of Blobs:

   - A Blob is read-only, and a MutableBlob is readable and writeable.
   - If a Blob is created from a raw pointer to data, the data will be copied.
     Otherwise if it's created from another Blob then none of the underlying
     data will be copied.
   - Blobs can be created as arbitrary subsets of existing Blobs without copying
     any underlying data.
   - The deletion of any Blob cannot affect any other Blob, even when Blobs are
     subsets of other Blobs and share the same underlying data. Underlying Blob
     data is freed automatically when all Blobs which refer to it are deleted.
   - A MutableBlob is the only object that can modify an instance of underlying
     data. MutableBlobs cannot be copied or created from MutableBlobs, but Blobs
     can be created from MutableBlobs (they are single-writer, multiple-reader). A
     MutableBlob always allocates and copies data instead of sharing with existing Blobs.
   - Blobs support clearing their data upon deallocation. This is enabled by
     setting the 'ScrubType' to something other than 'NONE' upon construction. All
     Blobs which are created from existing Blobs inherit this property.
*/

class Blob
{
 public:
  enum class ScrubType
  {
    NONE, ZEROS
  };
  enum class CompareType
  {
    DEFAULT, CONST
  };
  enum class Comparison
  {
    EQ, NE
  };
  typedef std::function<bool(const Blob &a, const Blob &b)> Comparator;
  typedef std::function<std::unique_ptr<std::string>(const Byte *data, U64 size)> Encoder;
  typedef std::function<std::unique_ptr<Blob>(const Byte *data, U64 size)> Decoder;

 public:
  Blob(U64 size = 0, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  Blob(const Byte *stream, U64 size, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  Blob(const char *stream, U64 size, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  Blob(const Blob &other, U64 size, U64 offset = 0);
  Blob(const Byte *data, U64 size, Decoder decoder);
  Blob(const std::string &data);
  Blob(const std::string &data, Decoder decoder);
  Blob(std::initializer_list<Blob> blobs, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  Blob(const Blob &) = default;
  Blob(Blob &&) = default;
  Blob &operator=(const Blob &) = default;
  Blob &operator=(Blob &&) = default;
  bool operator==(const Blob &other) const;
  bool operator!=(const Blob &other) const;
  const Byte &operator[](U64 index) const;
  Comparison compare(const Blob &other, CompareType compareType);
  void dataIs(const Byte *stream, U64 size, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  void dataIs(const char *stream, U64 size, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  void dataIsNull();
  const Byte *data() const;
  std::unique_ptr<std::string> data(Encoder encoder) const;
  U64 size() const;
  ScrubType scrubType() const;
  CompareType compareType() const;

 protected:
  static Container::ScrubType scrubberForType(ScrubType scrubType);
  static Comparator comparatorForType(CompareType compareType);
  std::shared_ptr<Container> container_;
  ScrubType scrubType_;
  CompareType compareType_;
  Comparator comparator_;
  const Byte *data_;
  U64 size_;
};

class MutableBlob : public Blob
{
 public:
  MutableBlob(U64 size, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  MutableBlob(const Byte *stream, U64 size, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  MutableBlob(const Blob &other, ScrubType scrubType = ScrubType::NONE,
    CompareType compareType = CompareType::DEFAULT);
  MutableBlob(const MutableBlob &) = delete;
  MutableBlob(MutableBlob &&) = default;
  MutableBlob &operator=(const MutableBlob &) = delete;
  MutableBlob &operator=(MutableBlob &&) = default;
  Byte &operator[](U64 index);
  Byte *data();
};

} // namespace Util

#endif // UTIL_BLOB_H

