/**
 * @file DetectorIDTest.cxx
 * @brief Test the operation of DetectorID class
 */
#include "Framework/catch.hpp"  //for TEST_CASE, REQUIRE, and other Catch2 macros

#include <sstream>
#include "DetDescr/DetectorID.h"  //headers defining what we will be testing
#include "DetDescr/DetectorIDInterpreter.h"  //headers defining what we will be testing
#include "DetDescr/EcalID.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "DetDescr/TrackerID.h"
#include "DetDescr/TrigScintID.h"

/**
 * Test for DetectorID function
 *
 * First argument is name of this test.
 * Second argument is tags to group tests together.
 */
TEST_CASE("DetectorID", "[DetDescr][functionality]") {
  using namespace ldmx;
  DetectorID did_null;
  DetectorID did_raw(0x40000000);
  DetectorID did_ecal(SD_ECAL, 0x1010);
  DetectorID did_hcal(SD_HCAL, 0x0010);
  DetectorID did_tk(SD_TRACKER_RECOIL, 0x0010);
  DetectorID did_ts(SD_TRIGGER_SCINT, 0x22);
  DetectorID did_ss(SD_SIM_SPECIAL, 0x24);

  CHECK_NOTHROW(DetectorIDInterpreter());

  SECTION("Detector ID") {
    REQUIRE(did_null.null());
    REQUIRE(did_raw.subdet() == EID_TRACKER);
    REQUIRE(did_ecal.subdet() == SD_ECAL);
    REQUIRE(did_ecal.raw() == 0x14001010);
    REQUIRE(did_ecal < did_raw);
    REQUIRE(did_ecal != did_null);
  }
  SECTION("EcalID") {
    EcalID eid_empty;
    EcalID eid_raw(0x14002020);

    REQUIRE_THROWS(EcalID(did_raw));
    REQUIRE_THROWS(EcalID(0x829018));
    REQUIRE_NOTHROW(EcalID(did_ecal));
    REQUIRE_NOTHROW(EcalID(did_ecal.raw()));

    EcalID eid(50, 15, 500);  // choose big values to check collisions

    REQUIRE(eid.raw() == 0x1464f1f4);

    REQUIRE(eid.layer() == 50);
    REQUIRE(eid.layer() == eid.getLayerID());
    REQUIRE(eid.module() == 15);
    REQUIRE(eid.module() == eid.getModuleID());
    REQUIRE(eid.cell() == 500);
    REQUIRE(eid.cell() == eid.getCellID());

    std::stringstream ss;
    ss << eid;

    REQUIRE(ss.str() == "Ecal(50,15,500[92,23])");

    DetectorIDInterpreter dii(eid);

    CHECK(dii.getFieldValue("layer") == 50);
    CHECK(dii.getFieldValue("module") == 15);
    CHECK(dii.getFieldValue("cell") == 500);

    dii.setFieldValue("layer", 23);
    dii.setFieldValue("module", 13);
    dii.setFieldValue("cell", 223);

    eid = EcalID(dii.getId());

    CHECK(eid.layer() == 23);
    CHECK(eid.module() == 13);
    CHECK(eid.cell() == 223);
  }
  SECTION("HcalID") {
    HcalID hid_empty;
    HcalID hid_raw(0x18002020);

    REQUIRE_THROWS(HcalID(did_raw));
    REQUIRE_THROWS(HcalID(0x829018));
    REQUIRE_NOTHROW(HcalID(did_hcal));
    REQUIRE_NOTHROW(HcalID(did_hcal.raw()));

    HcalID hid(HcalID::LEFT, 250,
               245);  // choose big values to check collisions

    REQUIRE(hid.raw() == 0x1813e8f5);

    REQUIRE(hid.layer() == 250);
    REQUIRE(hid.layer() == hid.getLayerID());
    REQUIRE(hid.section() == HcalID::LEFT);
    REQUIRE(hid.section() == hid.getSection());
    REQUIRE(hid.strip() == 245);
    REQUIRE(hid.strip() == hid.getStrip());

    std::stringstream ss;
    ss << hid;

    REQUIRE(ss.str() == "Hcal(4,250,245)");

    DetectorIDInterpreter dii(hid);

    CHECK(dii.getFieldValue("layer") == 250);
    CHECK(dii.getFieldValue("section") == 4);
    CHECK(dii.getFieldValue("strip") == 245);

    dii.setFieldValue("layer", 23);
    dii.setFieldValue("section", 1);
    dii.setFieldValue("strip", 223);

    hid = HcalID(dii.getId());

    CHECK(hid.layer() == 23);
    CHECK(hid.section() == 1);
    CHECK(hid.strip() == 223);
  }

  SECTION("TrackerID") {
    TrackerID tid_empty;
    TrackerID tid_raw(0x10002020);

    REQUIRE_THROWS(TrackerID(did_raw));
    REQUIRE_THROWS(TrackerID(0x829018));
    REQUIRE_NOTHROW(TrackerID(did_tk));
    REQUIRE_NOTHROW(TrackerID(did_tk.raw()));

    TrackerID tid(SD_TRACKER_TAGGER, 250,
                  30);  // choose big values to check collisions
    TrackerID tid2(SD_TRACKER_RECOIL, 220,
                   17);  // choose big values to check collisions

    REQUIRE(tid.raw() == 0x4001efa);
    REQUIRE(tid2.raw() == 0x100011dc);

    REQUIRE(tid.layer() == 250);
    REQUIRE(tid.module() == 30);

    REQUIRE(tid2.layer() == 220);
    REQUIRE(tid2.module() == 17);

    std::stringstream ss;
    ss << tid << tid2;

    REQUIRE(ss.str() == "Tagger(250,30)Recoil(220,17)");

    DetectorIDInterpreter dii(tid);

    CHECK(dii.getFieldValue("layer") == 250);
    CHECK(dii.getFieldValue("module") == 30);

    dii.setFieldValue("layer", 213);
    dii.setFieldValue("module", 23);

    tid = TrackerID(dii.getId());

    CHECK(tid.layer() == 213);
    CHECK(tid.module() == 23);
  }
  SECTION("TrigScintID") {
    TrigScintID ts_id_empty;
    TrigScintID ts_id_raw(0x08002020);

    REQUIRE_THROWS(TrigScintID(did_raw));
    REQUIRE_THROWS(TrigScintID(0x829018));
    REQUIRE_NOTHROW(TrigScintID(did_ts));
    REQUIRE_NOTHROW(TrigScintID(did_ts.raw()));

    TrigScintID ts_id(250, 245);  // choose big values to check collisions

    CHECK(ts_id.raw() == 0x800faf5);

    REQUIRE(ts_id.module() == 250);
    REQUIRE(ts_id.getModule() == 250);
    REQUIRE(ts_id.bar() == 245);
    REQUIRE(ts_id.bar() == ts_id.getBarID());

    std::stringstream ss;
    ss << ts_id;

    REQUIRE(ss.str() == "TrigScint(250,245)");

    DetectorIDInterpreter dii(ts_id);

    CHECK(dii.getFieldValue("bar") == 245);
    CHECK(dii.getFieldValue("module") == 250);

    dii.setFieldValue("bar", 213);
    dii.setFieldValue("module", 23);

    ts_id = TrigScintID(dii.getId());

    CHECK(ts_id.bar() == 213);
    CHECK(ts_id.module() == 23);
  }
  SECTION("SimSpecialID") {
    SimSpecialID ssid_empty;
    SimSpecialID ssid_raw(0x1C002020);

    REQUIRE_THROWS(SimSpecialID(did_raw));
    REQUIRE_THROWS(SimSpecialID(0x829018));
    REQUIRE_NOTHROW(SimSpecialID(did_ss));
    REQUIRE_NOTHROW(SimSpecialID(did_ss.raw()));

    SimSpecialID ssid = SimSpecialID::ScoringPlaneID(
        3210);  // choose big values to check collisions
    SimSpecialID ssid2(SimSpecialID::SimSpecialType(9), 0xFEDCB);

    CHECK(ssid.raw() == 0x1c400c8a);
    CHECK(ssid2.raw() == 0x1e4fedcb);

    REQUIRE(ssid.getSubtype() == SimSpecialID::SCORING_PLANE);
    REQUIRE(ssid2.getSubtype() == 9);

    REQUIRE(ssid.plane() == 3210);
    REQUIRE(ssid2.plane() == -1);
    REQUIRE(ssid.subtypePayload() == 3210);
    REQUIRE(ssid2.subtypePayload() == 0xfedcb);

    std::stringstream ss;
    ss << ssid << ssid2;

    REQUIRE(ss.str() ==
            "SimSpecial(ScoringPlane 3210)SimSpecial(Type 9,1043915)");

    DetectorIDInterpreter dii(ssid);

    CHECK(dii.getFieldValue("subtype") == 1);
    CHECK(dii.getFieldValue("payload") == 3210);
  }
}
