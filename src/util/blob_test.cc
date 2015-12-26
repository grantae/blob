#include "gtest/gtest.h"
#include "util/blob.h"
#include <cstring>

using namespace Util;
using std::string;
using std::shared_ptr;
using std::unique_ptr;

static Byte buf1[4] = {0x1, 0x2, 0x3, 0x4};
static Byte buf2[3] = {0x1, 0x2, 0x3};
static Byte buf3[3] = {0xa, 0xb, 0xc};

TEST(BlobTest, Copy) {
  // Construct
  Blob blob1(buf1, 4);
  EXPECT_EQ(blob1.size(), 4UL);
  EXPECT_NE(blob1.data(), buf1);

  // dataIs()
  Blob blob2;
  EXPECT_EQ(blob2.size(), 0UL);
  blob2.dataIs(buf1, 4);
  EXPECT_EQ(blob2.size(), 4UL);
  EXPECT_NE(blob2.data(), buf1);
}

TEST(BlobTest, NoCopy) {
  // Copy construct
  Blob blob1(buf1, 4);
  Blob blob2(blob1);
  EXPECT_EQ(blob1.data(), blob2.data());
  EXPECT_TRUE(blob1 == blob2);
  Blob blob3(blob1, 3, 1);
  EXPECT_EQ(blob1.data()+1, blob3.data());
  EXPECT_EQ(blob1.size()-1, blob3.size());
  EXPECT_TRUE(blob1[1] == blob3[0]);
  EXPECT_TRUE(blob1[2] == blob3[1]);
  EXPECT_TRUE(blob1[3] == blob3[2]);
  Blob blob4(blob1, 3, 0);
  EXPECT_EQ(blob1.data(), blob4.data());
  EXPECT_EQ(blob1.size()-1, blob4.size());
  EXPECT_TRUE(blob1[0] == blob4[0]);
  EXPECT_TRUE(blob1[1] == blob4[1]);
  EXPECT_TRUE(blob1[2] == blob4[2]);

  // Copy assignment
  Blob blob5 = blob2;
  EXPECT_EQ(blob2.data(), blob5.data());
  EXPECT_TRUE(blob2 == blob5);
  Blob blob6 = blob3;
  EXPECT_EQ(blob3.data(), blob6.data());
  EXPECT_TRUE(blob3 == blob6);
  Blob blob7 = blob4;
  EXPECT_EQ(blob4.data(), blob7.data());
  EXPECT_TRUE(blob4 == blob7);
}

TEST(BlobTest, Equals) {
  Blob b1(buf1, 4);
  Blob b2(buf2, 3);
  Blob b3(buf3, 3);
  Blob b4(buf3+1, 2);
  Blob b5(buf3, 2);
  Blob b6(buf1, 3);
  Blob b7(buf3, 3);

  // Different data, different lengths, different pointers
  EXPECT_FALSE(b1 == b3);
  EXPECT_TRUE(b1 != b3);

  // Different data, different lengths, same pointers
  EXPECT_FALSE(b1 == b4);
  EXPECT_TRUE(b1 != b4);

  // Different data, same lengths, different pointers
  EXPECT_FALSE(b2 == b3);
  EXPECT_TRUE(b2 != b3);

  // Different data, same lengths, same pointers
  // (not possible)

  // Same data, different lengths, different pointers
  EXPECT_FALSE(b1 == b2);
  EXPECT_TRUE(b1 != b2);

  // Same data, different lengths, same pointers
  EXPECT_FALSE(b3 == b5);
  EXPECT_TRUE(b3 != b5);

  // Same data, same lengths, different pointers
  EXPECT_FALSE(b2 != b6);
  EXPECT_TRUE(b2 == b6);

  // Same data, same lengths, same pointers
  EXPECT_FALSE(b3 != b7);
  EXPECT_TRUE(b3 == b7);
}

TEST(BlobTest, ArrayOperator) {
  Blob b1(buf1, 4);
  EXPECT_EQ(1, b1[0]);
  EXPECT_EQ(2, b1[1]);
  EXPECT_EQ(3, b1[2]);
  EXPECT_EQ(4, b1[3]);
}

TEST(BlobTest, SizeContents) {
  Blob b1;
  EXPECT_EQ(0UL, b1.size());
  b1.dataIs(buf1, 4);
  EXPECT_EQ(4UL, b1.size());
  EXPECT_EQ(0, memcmp((const void *)b1.data(), (void *)buf1, 4));

  Blob b2(buf3, 3);
  EXPECT_EQ(3UL, b2.size());
  EXPECT_EQ(0, memcmp((const void *)b2.data(), (void *)buf3, 3));
}

TEST(BlobTest, Reassignment) {
  Blob b1(buf1, 4);
  EXPECT_EQ(4UL, b1.size());
  EXPECT_EQ(0, memcmp((const void *)b1.data(), (void *)buf1, 4));
  Blob b2 = b1;
  b1.dataIs(buf3, 3);
  EXPECT_EQ(3UL, b1.size());
  EXPECT_EQ(0, memcmp((const void *)b1.data(), (void *)buf3, 3));
  EXPECT_EQ(4UL, b2.size());
  b2.dataIsNull();
  EXPECT_EQ(0UL, b2.size());
}

TEST(BlobTest, MutableWrite) {
  MutableBlob b1(buf1, 4);
  EXPECT_EQ(4UL, b1.size());
  EXPECT_EQ(0, memcmp((const void *)b1.data(), (void *)buf1, 4));
  *b1.data() = 0xe;
  b1[1] = 0xf;
  EXPECT_EQ(0xe, b1[0]);
  EXPECT_EQ(0xf, b1[1]);
  EXPECT_EQ(0x3, b1[2]);
  EXPECT_EQ(0x4, b1[3]);
}

