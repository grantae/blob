#include "util/byte_encoders.h"
#include "util/blob.h"
#include <unistd.h>
#include <cstring>
#include <iostream>
using std::string;
using std::cout;
using std::endl;
using std::unique_ptr;
using Util::Blob;
using Util::MutableBlob;
using Util::encode_hex;
using Util::encode_base58;
using Util::encode_base64;
using Util::encode_string;

static void blockPrint(const string &msg, U32 offset, U32 cols)
{
  for (U64 i = 0; i < msg.size(); i++) {
    if ((i % (cols - offset)) == 0) {
      if (i != 0) {
        cout << endl;
      }
      for (U64 j = 0; j < offset; j++) {
        cout << " ";
      }
    }
    cout << msg[i];
  }
  cout << endl;
}

static void example_1()
{
  // Create a Blob from existing data (data is copied)
  char data[50];
  for (unsigned long i = 0; i < sizeof(data); i++) {
    data[i] = (char)i;
  }
  Blob b1(data, sizeof(data));

  // Create a Blob from another Blob (no data copying, automatic memory management)
  Blob b2 = b1;

  // Create a subset Blob with the first 32 bytes of b2
  Blob b3(b2, 32, 0);

  // Print a Blob using various encoders
  unique_ptr<string> bin = b3.data(Util::encode_bin);
  cout << "Blob printed in binary:" << endl;
  blockPrint(*bin, 4, 80);
  unique_ptr<string> hex = b3.data(Util::encode_hex);
  cout << "Blob printed in hex:" << endl;
  blockPrint(*hex, 4, 80);
  unique_ptr<string> base58 = b3.data(Util::encode_base58);
  cout << "Blob printed in base58:" << endl;
  blockPrint(*base58, 4, 80);
  cout << "Blob printed in base 62:" << endl;
  unique_ptr<string> base62 = b3.data(Util::encode_base62);
  blockPrint(*base62, 4, 80);
  cout << "Blob printed in base 64:" << endl;
  unique_ptr<string> base64 = b3.data(Util::encode_base64);
  blockPrint(*base64, 4, 80);

  // Create a Blob that will zero its data upon destruction
  Blob b4(data, sizeof(data), Blob::ScrubType::ZEROS);

  // Compare Blob contents for equality
  b3 == b4;

  // Create a Blob with constant-time comparison
  Blob b5(data, sizeof(data), Blob::ScrubType::ZEROS, Blob::CompareType::CONST);
  b5 == b4;

  // Compare Blobs for equality using a specified comparator
  b5.compare(b4, Blob::CompareType::DEFAULT);

  // Access a single byte of a Blob
  b5[7];

  // Access the pointer to a Blob's data
  b5.data();

  // Find the size (in bytes) of a Blob
  b5.size();

  // Create a Blob with writable data
  MutableBlob b6(b5.size());

  // Copy data into a MutableBlob
  const Byte *cdata = b5.data();
  Byte *mdata = b6.data();
  memcpy((void *)mdata, (const void *)cdata, b6.size());

  // Directly modify a byte of a MutableBlob
  b6[0] = 0;

  // Create a Blob from a MutableBlob
  Blob b7(b6);
}

int main()
{
  example_1();
  return 0;
}

