
#include "Framework/catch.hpp"

#include "Packing/RawDataFile/Reader.h"
#include "Packing/RawDataFile/Writer.h"

/**
 * Can we read/write binary files with our reader/writers
 */
TEST_CASE("BinaryIO", "[Packing][functionality]") {
  std::string test_file{"test.raw"};

  // test_vec and test_wide have the same data content
  // but will be stored differently depending on endian-ness of system
  std::vector<uint16_t> test_vec = { 0xFFFF, 0x00FF, 0xFF00, 0xF0F0 };
  uint64_t test_wide = 0xFFFF00FFFF00F0F0;

  SECTION("Write") {
    packing::rawdatafile::Writer w(test_file);

    CHECK(w << test_vec);
    CHECK(w << test_wide);
  }

  SECTION("Read") {
    packing::rawdatafile::Reader r(test_file);

    std::vector<uint16_t> read_vec;
    uint64_t read_wide;
    CHECK(r.read(read_vec, 4));
    CHECK(r >> read_wide);
    CHECK(read_vec == test_vec);
    CHECK(read_wide == test_wide);
  }

}
