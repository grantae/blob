#ifndef UTIL_BYTE_ENCODERS_H
#define UTIL_BYTE_ENCODERS_H

#include "util/fixed_types.h"
#include "util/make_unique.h"
#include "util/blob.h"
#include <cstddef>  // C++11 include fix for GMP up to 5.1.3
#include <gmpxx.h>
#include <string>
#include <functional>
#include <memory>

namespace Util {


// *** String Encoder ***
auto encode_string = [] (const Byte *data, U64 size)
{
  return Util::make_unique<std::string>((const char *)data, size);
};


// *** Base 2 (Binary) Encoder ***
auto encode_bin = [] (const Byte *data, U64 size)
{
  const U64 outputSize = 8 * size;
  std::unique_ptr<std::string> outputPtr(new std::string);
  std::string &output = *outputPtr.get();
  output.reserve(outputSize);

  Byte bin[8];
  for (U64 i = 0; i < size; i++) {
    bin[0] = 0x30 | (data[i] >> 7);
    bin[1] = 0x30 | ((data[i] >> 6) & 0x1);
    bin[2] = 0x30 | ((data[i] >> 5) & 0x1);
    bin[3] = 0x30 | ((data[i] >> 4) & 0x1);
    bin[4] = 0x30 | ((data[i] >> 3) & 0x1);
    bin[5] = 0x30 | ((data[i] >> 2) & 0x1);
    bin[6] = 0x30 | ((data[i] >> 1) & 0x1);
    bin[7] = 0x30 | (data[i] & 0x1);
    output.append((const char *)bin, 8);
  }
  return outputPtr;
};

auto decode_bin = [] (const Byte *data, U64 size)
{
  size &= ~0x7UL;
  U64 outSize = size / 8;

  MutableBlob outBlob(outSize);
  Byte *out = outBlob.data();
  U64 inPos = 0;

  for (U64 i = 0; i < outSize; i++) {
    out[i]  = (data[inPos++] & 0x1) << 7;
    out[i] |= (data[inPos++] & 0x1) << 6;
    out[i] |= (data[inPos++] & 0x1) << 5;
    out[i] |= (data[inPos++] & 0x1) << 4;
    out[i] |= (data[inPos++] & 0x1) << 3;
    out[i] |= (data[inPos++] & 0x1) << 2;
    out[i] |= (data[inPos++] & 0x1) << 1;
    out[i] |=  data[inPos++] & 0x1;
  }
  return Util::make_unique<Blob>(outBlob);
};


// *** Base 16 (Hex) Encoder ***
auto encode_hex = [] (const Byte *data, U64 size)
{
  static const char hex[16] =
      {'0', '1', '2', '3', '4', '5', '6', '7',
       '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  const U64 outputSize = 2 * size;
  std::unique_ptr<std::string> outputPtr(new std::string());
  std::string &output = *outputPtr.get();
  output.reserve(outputSize);

  for (U64 i = 0; i < size; i++) {
    output.push_back(hex[(data[i] >> 4)]);
    output.push_back(hex[(data[i] & 0xf)]);
  }
  return outputPtr;
};

auto decode_hex = [] (const Byte *data, U64 size)
{
  // Ignore a dangling nibble if present
  size &= ~0x1UL;

  U64 outSize = size / 2;
  MutableBlob outBlob(outSize);
  Byte *out = outBlob.data();
  U64 inPos = 0;

  for (U64 i = 0; i < outSize; i++) {
    Byte nib1 = data[inPos++];
    Byte nib2 = data[inPos++];
    nib1 -= (nib1 > 0x40) ? 0x37 : 0x30;
    nib2 -= (nib2 > 0x40) ? 0x37 : 0x30;
    out[i] = (nib1 << 4) | nib2;
  }
  return make_unique<Blob>(outBlob);
};


// *** Base 58 Encoder (Bitcoin-like encoding) ***
auto encode_base58 = [] (const Byte *data, U64 size)
{
  static const char *b58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

  // Count leading zeros
  U64 leadingZeros = 0;
  for (U64 i = 0; i < size; i++) {
    if (data[i] == 0x00) {
      leadingZeros += 1;
    }
    else {
      break;
    }
  }
  // Ouput reserve size: log(256) / log(58), rounded up
  const U64 reserve = (size - leadingZeros) * 138 / 100 + 1 + leadingZeros;

  std::unique_ptr<Byte[]> outPtr(new Byte[reserve]);
  Byte *out = outPtr.get();
  Byte *pos = &out[reserve];

  mpz_class d("58");  // divisor
  mpz_class n, r;     // dividend and remainder
  mpz_import(n.get_mpz_t(), size, 1, 1, 0, 0, data);

  while (mpz_get_ui(n.get_mpz_t()) != 0U) {
    pos--;
    mpz_fdiv_qr(n.get_mpz_t(), r.get_mpz_t(), n.get_mpz_t(), d.get_mpz_t());
    *pos = (Byte) b58[mpz_get_ui(r.get_mpz_t())];
  }
  for (U64 i = 0; i < leadingZeros; i++) {
    pos--;
    *pos = (Byte) b58[0];
  }
  U64 encodedSize = (U64)(&out[reserve] - pos);
  return make_unique<std::string>((char *)pos, encodedSize);
};

/* TODO see if this or the other is faster
auto decode_base58 = [] (const Byte *data, U64 size)
{
  static const char *b58 =
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\xff\xff\xff\xff\xff\xff\xff\x09\x0A\x0B\x0C"
    "\x0D\x0E\x0F\x10\xff\x11\x12\x13\x14\x15\xff\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E"
    "\x1F\x20\xff\xff\xff\xff\xff\xff\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\xff"
    "\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\xff\xff\xff\xff\xff";

  mpz_class p(0); // product and multiplicand

  // Apply "p = p * 58 + n" for all n in 'data'
  for (U64 i = 0; i < size; i++) {
    Byte n = (Byte) b58[(data[i] & 0x7f) - 0x31];
    mpz_mul_ui(p.get_mpz_t(), p.get_mpz_t(), 58);
    mpz_add_ui(p.get_mpz_t(), p.get_mpz_t(), n);
  }

  // Allocate and copy the output
  U64 outSize = (mpz_sizeinbase(p.get_mpz_t(), 2) + 7) / 8;
  Util::MutableBlob out(outSize);
  mpz_export((void *)out.data(), nullptr, 1, 1, 0, 0, p.get_mpz_t());

  // Return a unique pointer to a Blob
  return make_unique<Util::Blob>(out);
};
*/

auto decode_base58 = [] (const Byte *data, U64 size)
{
  // Count leading zeros
  U64 leadingZeros = 0;
  for (U64 i = 0; i < size; i++) {
    if (data[i] == (Byte) '1') {
      leadingZeros += 1;
    }
    else {
      break;
    }
  }
  mpz_class p(0); // product and multiplicand

  // Apply "p = p * 58 + n" for all n in 'data'
  for (U64 i = 0; i < size; i++) {
    Byte n = data[i];
    if (n > 0x6c) {
      n -= 0x41;
    }
    else if (n > 0x60) {
      n -= 0x40;
    }
    else if (n > 0x4f) {
      n -= 0x3a;
    }
    else if (n > 0x49) {
      n -= 0x39;
    }
    else if (n > 0x40) {
      n -= 0x38;
    }
    else {
      n -= 0x31;
    }
    mpz_mul_ui(p.get_mpz_t(), p.get_mpz_t(), 58);
    mpz_add_ui(p.get_mpz_t(), p.get_mpz_t(), n);
  }

  // Allocate and copy the output
  U64 outSize = (mpz_sizeinbase(p.get_mpz_t(), 2) + 7) / 8 + leadingZeros;
  MutableBlob out(outSize);
  Byte *outData = out.data();
  for (U64 i = 0; i < leadingZeros; i++) {
    *outData++ = 0x00;
  }
  mpz_export((void *)outData, nullptr, 1, 1, 0, 0, p.get_mpz_t());

  // Return a unique pointer to a Blob
  return make_unique<Blob>(out);
};


// *** Base 62 Encoder ***
auto encode_base62 = [] (const Byte *data, U64 size)
{
  static const char *b62 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  // Count leading zeros
  U64 leadingZeros = 0;
  for (U64 i = 0; i < size; i++) {
    if (data[i] == 0x00) {
      leadingZeros += 1;
    }
    else {
      break;
    }
  }
  // Output reserve size: log(256) / log(62), rounded up
  const U64 reserve = (size - leadingZeros) * 137 / 100 + 1 + leadingZeros;

  std::unique_ptr<Byte[]> outPtr(new Byte[reserve]);
  Byte *out = outPtr.get();
  Byte *pos = &out[reserve];

  mpz_class d("62");  // divisor
  mpz_class n, r;     // dividend and remainder
  mpz_import(n.get_mpz_t(), size, 1, 1, 0, 0, data);

  while (mpz_get_ui(n.get_mpz_t()) != 0U) {
    pos--;
    mpz_fdiv_qr(n.get_mpz_t(), r.get_mpz_t(), n.get_mpz_t(), d.get_mpz_t());
    *pos = (Byte) b62[mpz_get_ui(r.get_mpz_t())];
  }
  for (U64 i = 0; i < leadingZeros; i++) {
    pos--;
    *pos = (Byte) b62[0];
  }
  U64 encodedSize = (U64)(&out[reserve] - pos);
  return make_unique<std::string>((char *)pos, encodedSize);
};

auto decode_base62 = [] (const Byte *data, U64 size)
{
  // Count leading zeros
  U64 leadingZeros = 0;
  for (U64 i = 0; i < size; i++) {
    if (data[i] == (Byte) '0') {
      leadingZeros += 1;
    }
    else {
      break;
    }
  }
  mpz_class p(0); // product and multiplicand

  // Apply "p = p * 62 + n" for all n in 'data'
  for (U64 i = 0; i < size; i++) {
    Byte n = data[i];
    n -= ((n > 0x60) ? 0x3D : ((n > 0x40) ? 0x37 : 0x30));
    mpz_mul_ui(p.get_mpz_t(), p.get_mpz_t(), 62);
    mpz_add_ui(p.get_mpz_t(), p.get_mpz_t(), n);
  }

  // Allocate and copy the output
  U64 outSize = (mpz_sizeinbase(p.get_mpz_t(), 2) + 7) / 8 + leadingZeros;
  Util::MutableBlob out(outSize);
  Byte *outData = out.data();
  for (U64 i = 0; i < leadingZeros; i++) {
    *outData++ = 0x00;
  }
  mpz_export((void *)outData, nullptr, 1, 1, 0, 0, p.get_mpz_t());

  // Return a unique pointer to a Blob
  return make_unique<Blob>(out);
};

// *** Base 64 Encoder (without padding) ***
auto encode_base64 = [] (const Byte *data, U64 size)
{
  static const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  U64 i = 0;
  U64 j = 0;
  Byte array3[3];
  Byte array4[4];

  U64 outputSize = ((size + ((size % 3) ? (3 - (size % 3)) : 0)) / 3) * 4;
  std::unique_ptr<std::string> outputPtr(new std::string());
  std::string &output = *outputPtr.get();
  output.reserve(outputSize);

  while (size--) {
    array3[i++] = *(data++);
    if (i == 3) {
      i = 0;
      array4[0] = (array3[0] & 0xfc) >> 2;
      array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
      array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
      array4[3] = array3[2] & 0x3f;

      output += base64[array4[0]];
      output += base64[array4[1]];
      output += base64[array4[2]];
      output += base64[array4[3]];
    }
  }
  if (i != 0) {
    for (j = i; j < 3; j++) {
      array3[j] = 0;
    }
    array4[0] = (array3[0] & 0xfc) >> 2;
    array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
    array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
    array4[3] = array3[2] & 0x3f;

    for (j = 0; j < i + 1; j++) {
      output += base64[array4[j]];
    }
    //while (i++ < 3) {
    //    output += '=';
    //}
  }
  return outputPtr;
};

auto decode_base64 = [] (const Byte *data, U64 size)
{
  static const char *b64 =
    "\x3E\xff\xff\xff\x3F\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\xff\xff\xff\xff\xff"
    "\xff\xff\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11"
    "\x12\x13\x14\x15\x16\x17\x18\x19\xff\xff\xff\xff\xff\xff\x1A\x1B\x1C\x1D\x1E\x1F"
    "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33";

  U64 groups = size >> 2;
  U64 stragglers = size & 0x3;
  U64 outSize = (groups * 3) + ((stragglers >= 2) ? stragglers - 1 : 0);
  MutableBlob outBlob(outSize);
  Byte *out = outBlob.data();

  for (U64 i = 0; i < groups; i++) {
    Byte A = *data++ - 0x2b;
    Byte B = *data++ - 0x2b;
    Byte C = *data++ - 0x2b;
    Byte D = *data++ - 0x2b;
    A = (A < 80) ? (Byte) b64[A] : 0x00;
    B = (B < 80) ? (Byte) b64[B] : 0x00;
    C = (C < 80) ? (Byte) b64[C] : 0x00;
    D = (D < 80) ? (Byte) b64[D] : 0x00;
    *out++ = (A << 2) | (B >> 4);
    *out++ = (B << 4) | (C >> 2);
    *out++ = (C << 6) | D;
  }
  if (stragglers >= 2) {
    Byte A = *data++ - 0x2b;
    Byte B = *data++ - 0x2b;
    A = (A < 80) ? (Byte) b64[A] : 0x00;
    B = (B < 80) ? (Byte) b64[B] : 0x00;
    *out++ = (A << 2) | (B >> 4);
    if (stragglers > 2) {
      Byte C = *data - 0x2b;
      C = (C < 80) ? (Byte) b64[C] : 0x00;
      *out = (B << 4) | (C >> 2);
    }
  }
  return make_unique<Blob>(outBlob);
};

} // namespace Util

#endif // UTIL_BYTE_ENCODERS_H

