# Blob

Blob is a container class for data. It is useful when you want to pass data
between various classes or APIs because it manages its own memory. It is
fast if you want to work with subsets (or "chunks") of data because it
won't ever copy the underlying data, and subsets can never affect other
subsets (e.g., when they are deleted). Blobs also support custom comparators
and deallocators which allow for secure constant-time comparisons and
automatic data scrubbing.

## Quick Start

1. Clone the repo: `git clone https://github.com/grantae/blob.git`.
2. Build and test: `make test`.

Alternatively you can copy the directory `src/util` to your project.

## Example usage

1. Create a Blob that is read/write:

    ```
    Util::MutableBlob blob(1024);     // 1 KB size
    Byte *data = blob.data();         // Writable pointer to unsigned char
    U64 size = blob.size();           // Size of data (64-bit)
    ```
2. Create a Blob from an existing Blob:

    ```
    Util::MutableBlob blob1(1024);
    Util::Blob blob2(blob1);          // Read-only
    ```

3. Create a Blob from a raw pointer (data will be copied with this constructor):

    ```
    U64 size = 2048;
    Byte data = new Byte[size];
    Util::Blob(data, size);
    ```

4. Create a Blob as a subset of another Blob:

    ```
    Util::Blob blob1;                 // Some Blob of size 100
    Util::Blob blob2(blob1, 90, 10);  // The last 90 bytes of 'blob1'
    ```

5. Create a Blob that will zero its data upon deallocation:

    ```
    Byte *data;                       // Pre-existing data
    U64 size;                         // Size in bytes of 'data'
    Util::Blob blob(data, size, Util::Blob::ScrubType::ZEROS);
    ```

6. Create a Blob that will zero its data and do constant-time comparisons:

    ```
    Byte *data;                       // Pre-existing data
    U64 size;                         // Size in bytes of 'data'
    Util::Blob blob(data, size, Util::Blob::ScrubType::ZEROS,
      Util::Blob::CompareType::CONST);
    ```

7. Print a Blob in base-64:

    ```
    Util::Blob blob;                  // Some Blob of non-zero size
    std::unique_ptr<std::string> b64 = blob.data(Util::encode_base64);
    std::cout << *b64 << std::endl;
    ```

See more examples in [main.cc](https://github.com/grantae/blob/blob/master/src/main.cc)

## Requirements

* A C++11 (or later) compiler
* [GMP](https://gmplib.org) (GNU Multiple Precision Arithmetic Library) for
data conversion routines (e.g. to base 58).
* [GNU Make](https://www.gnu.org/software/make/) is required to build the
example code and tests.

## Creators

**Grant Ayers**

* <https://github.com/grantae>

## Copyright and license

Code and documentation copyright 2015 Grant Ayers. Code released under
[the MIT license](https://github.com/grantae/blob/blob/master/LICENSE)
