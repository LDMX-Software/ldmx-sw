
#include "TTree.h"

#include "Framework/catch.hpp"

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventHeader.h"
#include "Framework/RunHeader.h"

#include "Packing/Utility/Reader.h"
#include "Packing/Utility/Writer.h"
#include "Packing/RawDataFile/SubsystemPacket.h"
#include "Packing/RawDataFile/EventPacket.h"
#include "Packing/RawDataFile/File.h"

namespace packing {
namespace test {


}  // test
}  // packing

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

  SECTION("Event Packet") {
    std::string test_file{"event_packet_test.raw"};

    uint32_t event{420};
    std::map<uint16_t,std::vector<uint32_t>> unwrapped_data = {
      {0xFAFA,{0xAAAAAAAA,0xBBBBBBBB,0xCCCCCCCC,0xDDDDDDDD,0xDEDEDEDE,0xFEDCBA98}},
      {0xACDC,{0xFEBBBBEF,0x00112233}}
    };
      
    SECTION("Write") {
      packing::utility::Writer w(test_file);
  
      packing::rawdatafile::EventPacket ep(event,unwrapped_data);

      CHECK(w << ep);
    }
  
    SECTION("Read") {
      packing::utility::Reader r(test_file);
      packing::rawdatafile::EventPacket ep;

      CHECK(r >> ep);
      CHECK(ep.id() == event);

      for(auto& subsys : ep.data()) {
        REQUIRE(unwrapped_data.find(subsys.id()) != unwrapped_data.end());
        CHECK(unwrapped_data.at(subsys.id()) == subsys.data());
      }
    }
  }

  SECTION("Entire File") {
    std::string ecal_object_name("ecalDummyRaw"), hcal_object_name("hcalDummyRaw"),
      tracker_object_name("trackerDummyRaw"), triggerpad_object_name("triggerpadDummyRaw");
    framework::config::Parameters ps;
    ps.addParameter("filename", std::string("file_test.raw"));
    ps.addParameter("ecal_object_name", ecal_object_name);
    ps.addParameter("hcal_object_name", hcal_object_name);
    ps.addParameter("tracker_object_name", tracker_object_name);
    ps.addParameter("triggerpad_object_name", triggerpad_object_name);
    ps.addParameter("pass_name", std::string());
    ps.addParameter("skip_unavailable", true);
    ps.addParameter("verify_checksum", false);

    std::vector<uint32_t> data = {
        0xAAAAAAAA,
        0xBBBBBBBB,
        0xCCCCCCCC,
        0xDDDDDDDD,
        0xDEDEDEDE,
        0xFEDCBA98
    };
    int i_event{420};
    int n_events{1};
    int run{1666};

    SECTION("Write") {
      ps.addParameter("is_output", true);
      packing::rawdatafile::File f(ps);

      ldmx::RunHeader rh(run);
      f.writeRunHeader(rh);

      TTree dummy("dummy","dummy");
      framework::Event event("testwrite");
      event.setOutputTree(&dummy);
      f.connect(event);
      
      for (int i{0}; i < n_events; i++) {
        auto& eh{event.getEventHeader()};
        eh.setEventNumber(i_event+i);

        event.add(ecal_object_name, data);
        event.add(hcal_object_name, data);
        event.add(triggerpad_object_name, data);

        REQUIRE(f.nextEvent());

        event.Clear();
        event.onEndOfEvent();
      }

      f.close();
    }

    SECTION("Read") {
      ps.addParameter("is_output", false);
      packing::rawdatafile::File f(ps);

      ldmx::RunHeader rh(run);
      f.writeRunHeader(rh);
      CHECK(rh.getIntParameter("raw_run") == run);

      TTree dummy("dummy","dummy");
      framework::Event event("testread");
      event.setOutputTree(&dummy);
      f.connect(event);

      for (int i{0}; i < n_events; i++) {
        auto& eh{event.getEventHeader()};
        eh.setEventNumber(i);

        REQUIRE(f.nextEvent());

        CHECK(eh.getEventNumber() == i_event+i);
        CHECK(data == event.getCollection<uint32_t>(ecal_object_name));
        CHECK(data == event.getCollection<uint32_t>(hcal_object_name));
        CHECK(data == event.getCollection<uint32_t>(triggerpad_object_name));
        CHECK_FALSE(event.exists(tracker_object_name));

        event.Clear();
        event.onEndOfEvent();
      }

      REQUIRE_FALSE(f.nextEvent());

      f.close();
    }
  }
}
