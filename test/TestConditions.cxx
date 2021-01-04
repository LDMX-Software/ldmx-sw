#include "Framework/catch.hpp"  //for TEST_CASE, REQUIRE, and other Catch2 macros

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include "Conditions/SimpleCSVTableProvider.h"
#include "Conditions/SimpleTableCondition.h"
#include "Conditions/SimpleTableStreamers.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/HcalID.h"
#include "Framework/ConfigurePython.h"
#include "Framework/EventHeader.h"
#include "Framework/Process.h"
#include "Framework/RunHeader.h"

namespace ldmx {
namespace test {

template <class T>
void matchesMeta(const T& a, const T& b) {
  REQUIRE(a.getColumnCount() == b.getColumnCount());
  REQUIRE(a.getRowCount() == b.getRowCount());
  REQUIRE(a.getColumnNames() == b.getColumnNames());
}

void matchesAll(const ldmx::DoubleTableCondition& a,
                const ldmx::DoubleTableCondition& b) {
  matchesMeta(a, b);
  for (unsigned int i = 0; i < a.getRowCount(); i++) {
    std::pair<unsigned int, std::vector<double>> ar = a.getRow(i);
    std::pair<unsigned int, std::vector<double>> br = b.getRow(i);
    REQUIRE(ar.first == br.first);
    for (unsigned int ic = 0; ic < a.getColumnCount(); ic++) {
      //	  std::cout << i << ',' << ic << "=>" << ar.second[ic] << " " <<
      //br.second[ic] << std::endl;
      REQUIRE(fabs(ar.second[ic] - br.second[ic]) /
                  std::max(1e-5, (ar.second[ic] + br.second[ic]) / 2) <
              1e-5);  // limited precision of course
    }
  }
}

void matchesAll(const ldmx::IntegerTableCondition& a,
                const ldmx::IntegerTableCondition& b) {
  matchesMeta(a, b);
  for (unsigned int i = 0; i < a.getRowCount(); i++)
    REQUIRE(a.getRow(i) == b.getRow(i));
}

using Catch::Matchers::Contains;

/**
 * Test for various items related to conditions
 *
 * What does this test?
 *  - Correct creation, filling, and access to various implementations of
 * conditions tables
 */

TEST_CASE("Conditions", "[Framework][Conditions]") {
  using namespace ldmx;

  // create a simple table

  std::vector<std::string> columns({"A", "Q", "V"});

  IntegerTableCondition itable("ITable", columns);

  for (int key = 100; key > 0; key -= 10) {
    EcalID id(1, 1, key);
    std::vector<int> vals;
    vals.push_back(key * 2);
    vals.push_back(key / 2);
    vals.push_back(key * key);
    itable.add(id.raw(), vals);
  }

  std::vector<std::string> columnsd({"SQRT", "EXP", "LOG"});
  DoubleTableCondition dtable("DTable", columnsd);

  for (int key = 1; key < 8; key += 2) {
    HcalID id(1, 1, key);
    std::vector<double> vals;
    vals.push_back(sqrt(key));
    vals.push_back(exp(key * 5));
    vals.push_back(log(key));
    dtable.add(id.raw(), vals);
  }

  SECTION("Testing simple table construction") {
    REQUIRE(itable.getRowCount() == 10);

    REQUIRE(itable.getColumnCount() == 3);

    EcalID id(1, 1, 20);
    REQUIRE(itable.get(id.raw(), 1) == 10);

    REQUIRE_THROWS_WITH(itable.add(2, std::vector<int>(2)),
                        Contains("columns into a table"));

    REQUIRE_THROWS_WITH(itable.add(id.raw(), std::vector<int>(3)),
                        Contains("existing id"));

    std::pair<unsigned int, std::vector<int>> row = itable.getRow(4);

    CHECK(row.first == 0x14021032);

    CHECK(row.second.size() == 3);

    CHECK(row.second.at(2) == (50 * 50));

    EcalID id2(1, 1, 60);

    CHECK(itable.getByName(id2.raw(), "Q") == 30);
  }
  SECTION("Testing CSV IO") {
    std::stringstream ss;
    ldmx::utility::SimpleTableStreamerCSV::store(itable, ss, true);

    std::string image1 = ss.str();
    const char* expected1 =
        "\"DetID\",id:\"subdetector\",id:\"layer\",id:\"module\",id:\"cell\","
        "\"A\",\"Q\",\"V\"\n0x1402100a,5,1,1,10,20,5,100\n0x14021014,5,1,1,20,"
        "40,10,400\n0x1402101e,5,1,1,30,60,15,900\n0x14021028,5,1,1,40,80,20,"
        "1600\n0x14021032,5,1,1,50,100,25,2500\n0x1402103c,5,1,1,60,120,30,"
        "3600\n0x14021046,5,1,1,70,140,35,4900\n0x14021050,5,1,1,80,160,40,"
        "6400\n0x1402105a,5,1,1,90,180,45,8100\n0x14021064,5,1,1,100,200,50,"
        "10000\n";

    CHECK(image1 == expected1);

    // std::cout << image1;

    std::stringstream ss_read1(image1);
    IntegerTableCondition itable2("ITable", columns);
    ldmx::utility::SimpleTableStreamerCSV::load(itable2, ss_read1);

    matchesAll(itable, itable2);

    // missing id column example
    std::string image2("B,A,Q,V\n0,40,50,100\n0,60,70,90\n");
    std::stringstream ss_read2(image2);
    REQUIRE_THROWS_WITH(
        ldmx::utility::SimpleTableStreamerCSV::load(itable2, ss_read2),
        Contains("Malformed CSV file with no DetId or subdetector column"));

    // missing column example
    std::string image3(
        "DetID,A,Q\n0x1402100a,20,5\n0x14021014,40,10\n0x1402101e,60,"
        "15\n0x14021028,80,20\n0x14021032,100,25\n0x1402103c,120,"
        "30\n0x14021046,140,35\n0x14021050,160,40\n0x1402105a,180,"
        "45\n0x14021064,200,50\n");
    std::stringstream ss_read3(image3);
    REQUIRE_THROWS_WITH(
        ldmx::utility::SimpleTableStreamerCSV::load(itable2, ss_read3),
        Contains("Missing column"));

    // varying line lengths example
    std::string image4(
        "DetID,A,Q,V\n0x1402100a,20,5,100\n0x14021014,40,10\n0x1402101e,60,15,"
        "900\n");
    std::stringstream ss_read4(image4);
    REQUIRE_THROWS_WITH(
        ldmx::utility::SimpleTableStreamerCSV::load(itable2, ss_read4),
        Contains("Mismatched number of columns (3!=4) on line 3"));
  }

  SECTION("Testing python static") {
    const char* cfg =
        "#!/usr/bin/python\n\nimport sys\n\nfrom LDMX.Framework import "
        "ldmxcfg\nfrom LDMX.Conditions import "
        "SimpleCSVTableProvider\n\np=ldmxcfg.Process('test')\np.testMode="
        "True\ncolumns=['A','B','C']\ncop=SimpleCSVTableProvider."
        "SimpleCSVIntegerTableProvider('test_table_python',columns)\ncop."
        "validForAllRows([10,45,129])";

    FILE* f = fopen("/tmp/test_cond.py", "w");
    fputs(cfg, f);
    fclose(f);

    ldmx::ConfigurePython cp("/tmp/test_cond.py", 0, 0);
    ldmx::ProcessHandle hp = cp.makeProcess();
    EventHeader cxt;
    hp->setEventHeader(&cxt);

    cxt.setRun(10);
    const IntegerTableCondition& iTable =
        hp->getConditions().getCondition<IntegerTableCondition>(
            "test_table_python");

    CHECK(iTable.getByName(292, "A") == 10);
    CHECK(iTable.getByName(2928184, "B") == 45);
    CHECK(iTable.getByName(82910, "C") == 129);
  }

  SECTION("Testing file loading") {
    std::ofstream fs("/tmp/dump_double.csv");
    std::stringstream ss;
    ldmx::utility::SimpleTableStreamerCSV::store(dtable, fs, true);
    ldmx::utility::SimpleTableStreamerCSV::store(dtable, ss, true);
    fs.close();
    //	std::cout << "Step 1" << std::endl << ss.str();

    const char* cfg =
        "#!/usr/bin/python\n\nimport sys\n\nfrom LDMX.Framework import "
        "ldmxcfg\nfrom LDMX.Conditions import "
        "SimpleCSVTableProvider\n\np=ldmxcfg.Process('test')\np.testMode="
        "True\ncolumns=['SQRT','EXP','LOG']\ncop=SimpleCSVTableProvider."
        "SimpleCSVDoubleTableProvider('test_table_file',columns)\ncop."
        "validForRuns('file:///tmp/dump_double.csv',0,100)\ncop.validForRuns('/"
        "tmp/dump_double.csv',101,120)\n";

    FILE* f = fopen("/tmp/test_cond.py", "w");
    fputs(cfg, f);
    fclose(f);

    ldmx::ConfigurePython cp("/tmp/test_cond.py", 0, 0);
    ldmx::ProcessHandle hp = cp.makeProcess();
    EventHeader cxt;
    hp->setEventHeader(&cxt);

    cxt.setRun(10);
    const DoubleTableCondition& fTable1 =
        hp->getConditions().getCondition<DoubleTableCondition>(
            "test_table_file");
    cxt.setRun(119);
    const DoubleTableCondition& fTable2 =
        hp->getConditions().getCondition<DoubleTableCondition>(
            "test_table_file");
    matchesAll(dtable, fTable1);
    matchesAll(dtable, fTable2);
  }

  /*
  SECTION( "Testing HTTP loading" ) {
      const char* cfg="#!/usr/bin/python\n\nimport sys\n\nfrom LDMX.Framework
  import ldmxcfg\nfrom LDMX.Conditions import
  SimpleCSVTableProvider\n\np=ldmxcfg.Process(\"test\")\np.testMode=True\ncolumns=[\"A\",\"Q\",\"V\"]\ncop=SimpleCSVTableProvider.SimpleCSVDoubleTableProvider(\"test_table_http\",columns)\ncop.validForever(\"http://webusers.physics.umn.edu/~jmmans/test_table.csv\")\n";

      FILE* f=fopen("/tmp/test_cond.py","w");
      fputs(cfg,f);
      fclose(f);

      ldmx::ConfigurePython cp("/tmp/test_cond.py",0,0);
      ldmx::ProcessHandle hp=cp.makeProcess();
      EventHeader cxt;
      hp->setEventHeader(&cxt);

      const IntegerTableCondition&
  httpTable=hp->getConditions().getCondition<IntegerTableCondition>("test_table_http");
      matchesAll(httpTable,itable);
  }
  */
}
}  // namespace test
}  // namespace ldmx
