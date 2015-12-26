#include "gtest/gtest.h"
#include "util/container.h"
#include <cstring>
#include <memory>

using namespace Util;
using std::unique_ptr;

TEST(ContainerTest, ScrubZero) {
  unique_ptr<Byte[]> bufPtr(new Byte[1024]);
  Byte *buf = bufPtr.get();
  Byte test = 0;

  // Aligned (8-byte)
  memset((void *)buf, 0xff, 1024);
  scrub_zeros(buf, 1024);
  for (U64 i = 0; i < 1024; i++) {
    test |= buf[i];
  }
  EXPECT_EQ(test, 0x00);

  // Unaligned
  memset((void *)buf, 0xff, 1024);
  scrub_zeros(buf, 1021);
  test = 0;
  for (U64 i = 0; i < 1021; i++) {
    test |= buf[i];
  }
  EXPECT_EQ(test, 0x00);
  EXPECT_EQ(buf[1021], 0xff);
  EXPECT_EQ(buf[1022], 0xff);
  EXPECT_EQ(buf[1023], 0xff);
}

