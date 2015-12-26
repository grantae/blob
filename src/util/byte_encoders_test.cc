#include "gtest/gtest.h"
#include "util/byte_encoders.h"
#include "util/blob.h"
#include <random>
#include <chrono>

using namespace Util;
using std::string;
using std::unique_ptr;


// Fixed test strings
static const string s1 = "Duis viverra pretium enim et fringilla.";
static const string s2 = "Aliquam egestas, sapien a placerat dapib";
static const string s3 = "Etiam dolor dui, tempor eget turpis a, ma";
static const string s4 = "Nulla rutrum laoreet eros ac mollis. Nulla";
static const Blob b1(s1);
static const Blob b2(s2);
static const Blob b3(s3);
static const Blob b4(s4);

// A zero-led fixed string
static const Blob b5("\x00\x00\x00\x54\x65\x73\x74", 7);

// Random numbers for a variety of Blob conversions
static unsigned seed = 1234;
static std::default_random_engine gen(seed);
static std::uniform_int_distribution<Byte> dist;
static const int nRandTests = 400;

// Generate a random-sized blob with random data
static Blob randomBlob()
{
  unsigned size = dist(gen);
  while (size == 0) {
    size = dist(gen);
  }
  MutableBlob blob(size);
  Byte *data = blob.data();

  for (unsigned i = 0; i < size; i++) {
    data[i] = dist(gen);
  }
  return blob;
}

// Return true if a decoder reverses an encoder
static bool testReversibility(Blob::Encoder enc, Blob::Decoder dec)
{
  for (int i = 0; i < nRandTests; i++) {
    Blob a = randomBlob();
    unique_ptr<string> s(a.data(enc));
    Blob b(*s, dec);
    if (b != a) {
      return false;
    }
  }
  return true;
}

// Return true if a decoder reverses an encoder and matches a given result
static bool testCorrectness(Blob::Encoder enc, Blob::Decoder dec, const Blob &data, const string &test)
{
  unique_ptr<string> dataString(data.data(enc));
  if (dataString->compare(test) != 0) {
    return false;
  }
  Blob testData(test, dec);
  return (testData == data);
}


// **** Tests for each byte encoder ****

TEST(ByteEncodersTest, String) {
  unique_ptr<string> e1 = b1.data(encode_string);
  EXPECT_EQ(0, e1->compare(s1));
  unique_ptr<string> e2 = b2.data(encode_string);
  EXPECT_EQ(0, e2->compare(s2));
  unique_ptr<string> e3 = b3.data(encode_string);
  EXPECT_EQ(0, e3->compare(s3));
  unique_ptr<string> e4 = b4.data(encode_string);
  EXPECT_EQ(0, e4->compare(s4));
}

TEST(ByteEncodersTest, Bin) {
  // Fixed strings
  string e1 = "01000100011101010110100101110011001000000111011001101001011101100110010101110010"
              "01110010011000010010000001110000011100100110010101110100011010010111010101101101"
              "00100000011001010110111001101001011011010010000001100101011101000010000001100110"
              "011100100110100101101110011001110110100101101100011011000110000100101110";
  string e2 = "01000001011011000110100101110001011101010110000101101101001000000110010101100111"
              "01100101011100110111010001100001011100110010110000100000011100110110000101110000"
              "01101001011001010110111000100000011000010010000001110000011011000110000101100011"
              "01100101011100100110000101110100001000000110010001100001011100000110100101100010";
  string e3 = "01000101011101000110100101100001011011010010000001100100011011110110110001101111"
              "01110010001000000110010001110101011010010010110000100000011101000110010101101101"
              "01110000011011110111001000100000011001010110011101100101011101000010000001110100"
              "01110101011100100111000001101001011100110010000001100001001011000010000001101101"
              "01100001";
  string e4 = "01001110011101010110110001101100011000010010000001110010011101010111010001110010"
              "01110101011011010010000001101100011000010110111101110010011001010110010101110100"
              "00100000011001010111001001101111011100110010000001100001011000110010000001101101"
              "01101111011011000110110001101001011100110010111000100000010011100111010101101100"
              "0110110001100001";
  string e5 = "00000000000000000000000001010100011001010111001101110100";
  EXPECT_TRUE(testCorrectness(encode_bin, decode_bin, b1, e1));
  EXPECT_TRUE(testCorrectness(encode_bin, decode_bin, b2, e2));
  EXPECT_TRUE(testCorrectness(encode_bin, decode_bin, b3, e3));
  EXPECT_TRUE(testCorrectness(encode_bin, decode_bin, b4, e4));
  EXPECT_TRUE(testCorrectness(encode_bin, decode_bin, b5, e5));

  // Reversibility of random strings
  EXPECT_TRUE(testReversibility(encode_bin, decode_bin));
}

TEST(ByteEncodersTest, Hex) {
  // Fixed strings
  string e1 = "447569732076697665727261207072657469756D20656E696D206574206672696E67696C6C612E";
  string e2 = "416C697175616D20656765737461732C2073617069656E206120706C616365726174206461706962";
  string e3 = "457469616D20646F6C6F72206475692C2074656D706F7220656765742074757270697320612C206D61";
  string e4 = "4E756C6C612072757472756D206C616F726565742065726F73206163206D6F6C6C69732E204E756C6C61";
  string e5 = "00000054657374";
  EXPECT_TRUE(testCorrectness(encode_hex, decode_hex, b1, e1));
  EXPECT_TRUE(testCorrectness(encode_hex, decode_hex, b2, e2));
  EXPECT_TRUE(testCorrectness(encode_hex, decode_hex, b3, e3));
  EXPECT_TRUE(testCorrectness(encode_hex, decode_hex, b4, e4));
  EXPECT_TRUE(testCorrectness(encode_hex, decode_hex, b5, e5));

  // Reversibility of random strings
  EXPECT_TRUE(testReversibility(encode_hex, decode_hex));
}

TEST(ByteEncodersTest, Base58) {
  // Fixed strings
  string e1 = "mhWBKkZKwPKwdtXhJUUCtnqyzQyhqrVx9vYpLNZxhZyzSKyKteqQ1";
  string e2 = "4FYT5y5d1ULwFXYyByigriJPACdxoqWNNn86SmaxYT3oXEprpcnJCnD";
  string e3 = "GETuYeY2tmLyKpXzAv6LjQkKMGhQwNFzRhhefy3wQjy6bhgizFAadoMJ";
  string e4 = "2JwwVuLR1Ajeaa5dmSMHYCfPLaywSzfqAq8tfnq8bxxduXs3iMsNfEk3aQ";
  string e5 = "1113A836b";
  EXPECT_TRUE(testCorrectness(encode_base58, decode_base58, b1, e1));
  EXPECT_TRUE(testCorrectness(encode_base58, decode_base58, b2, e2));
  EXPECT_TRUE(testCorrectness(encode_base58, decode_base58, b3, e3));
  EXPECT_TRUE(testCorrectness(encode_base58, decode_base58, b4, e4));
  EXPECT_TRUE(testCorrectness(encode_base58, decode_base58, b5, e5));

  // Reversibility of random strings
  EXPECT_TRUE(testReversibility(encode_base58, decode_base58));
}

TEST(ByteEncodersTest, Base62) {
  // Fixed strings
  string e1 = "1OPVE2tnoox6nwYZe1KAYO8ddXlYoIPG1d3vqyObL5ahbcTi5fCLW";
  string e2 = "5UyXhZA6QsoGueFiFs4u6Dzml9e1ALSWUSpsWTp2gdGFn8g4FVWodO";
  string e3 = "O6ebEZ2kiTa4vqoTPx7sVsxG8jH6ZBI5k6nAlCZZSRdRt66htgx6agb";
  string e4 = "1oRX89j89W5ABceJCg5JrnDzCg5ZZkVcsWWpNx4EV0QvJtNdYhsjzjc8H";
  string e5 = "0001Xp7Ke";
  EXPECT_TRUE(testCorrectness(encode_base62, decode_base62, b1, e1));
  EXPECT_TRUE(testCorrectness(encode_base62, decode_base62, b2, e2));
  EXPECT_TRUE(testCorrectness(encode_base62, decode_base62, b3, e3));
  EXPECT_TRUE(testCorrectness(encode_base62, decode_base62, b4, e4));
  EXPECT_TRUE(testCorrectness(encode_base62, decode_base62, b5, e5));

  // Reversibility of random strings
  EXPECT_TRUE(testReversibility(encode_base62, decode_base62));
}

TEST(ByteEncodersTest, Base64) {
  // Fixed strings
  string e1 = "RHVpcyB2aXZlcnJhIHByZXRpdW0gZW5pbSBldCBmcmluZ2lsbGEu";
  string e2 = "QWxpcXVhbSBlZ2VzdGFzLCBzYXBpZW4gYSBwbGFjZXJhdCBkYXBpYg";
  string e3 = "RXRpYW0gZG9sb3IgZHVpLCB0ZW1wb3IgZWdldCB0dXJwaXMgYSwgbWE";
  string e4 = "TnVsbGEgcnV0cnVtIGxhb3JlZXQgZXJvcyBhYyBtb2xsaXMuIE51bGxh";
  string e5 = "AAAAVGVzdA";
  EXPECT_TRUE(testCorrectness(encode_base64, decode_base64, b1, e1));
  EXPECT_TRUE(testCorrectness(encode_base64, decode_base64, b2, e2));
  EXPECT_TRUE(testCorrectness(encode_base64, decode_base64, b3, e3));
  EXPECT_TRUE(testCorrectness(encode_base64, decode_base64, b4, e4));
  EXPECT_TRUE(testCorrectness(encode_base64, decode_base64, b5, e5));

  // Reversibility of random strings
  EXPECT_TRUE(testReversibility(encode_base64, decode_base64));
}

