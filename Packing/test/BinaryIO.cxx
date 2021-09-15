
#include "Framework/catch.hpp"

#include "Packing/Utility/Reader.h"
#include "Packing/Utility/Writer.h"
#include "Packing/RawDataFile/SubsystemPacket.h"

/**
 * Can we read/write binary files with our reader/writers
 */
TEST_CASE("BinaryIO", "[Packing][functionality]") {
  SECTION("Unformatted Data") {
    std::string test_file{"test.raw"};
    // test_vec and test_wide have the same data content
    // but will be stored differently depending on endian-ness of system
    std::vector<uint16_t> test_vec = { 0xFFFF, 0x00FF, 0xFF00, 0xF0F0 };
    uint64_t test_wide = 0xFFFF00FFFF00F0F0;
  
    SECTION("Write") {
      packing::utility::Writer w(test_file);
  
      CHECK(w << test_vec);
      CHECK(w << test_wide);
    }
  
    SECTION("Read") {
      packing::utility::Reader r(test_file);
  
      std::vector<uint16_t> read_vec;
      uint64_t read_wide, dummy;
      CHECK(r.read(read_vec, 4));
      CHECK(r >> read_wide);
      CHECK_FALSE(r >> dummy);
      CHECK(r.eof());
      CHECK(read_vec == test_vec);
      CHECK(read_wide == test_wide);
    }
  }

  SECTION("Subsystem Packet") {
    std::string test_file{"subsystem_packet_test.raw"};

    uint32_t event{420};
    uint16_t id{0xFAFA};
    std::vector<uint32_t> data = {
      0xAAAAAAAA,
      0xBBBBBBBB,
      0xCCCCCCCC,
      0xDDDDDDDD,
      0xDEDEDEDE,
      0xFEDCBA98
    };
      
  
    SECTION("Write") {
      packing::utility::Writer w(test_file);
  
      packing::rawdatafile::SubsystemPacket sp(event,id,data);

      CHECK(w << sp);
    }
  
    SECTION("Read") {
      packing::utility::Reader r(test_file);
      packing::rawdatafile::SubsystemPacket sp;

      CHECK(r >> sp);
      CHECK(sp.id() == id);
      CHECK(sp.data() == data);
    }
  }
}
