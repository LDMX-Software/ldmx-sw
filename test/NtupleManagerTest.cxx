/**
 * @file FunctionalCoreTest.cxx
 * @brief Test the operation of Framework processing
 *
 * @author Tom Eichlersmith, University of Minnesota
 *
 */
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "Framework/NtupleManager.h"
#include "TFile.h"        //to open and check root files
#include "TTreeReader.h"  //to check output event files

/**
 * Test for NtupleManager
 *
 * We check that the NtupleManager can fill a TTree with
 * three entries of the following types.
 *  - bool, short, int, long, float, double
 *  - vectors of the above types
 */
TEST_CASE("Ntuple Manager Functions", "[Framework][functionality]") {
  const char* ntuple_file = "/tmp/test_ntuplemanager.root";

  TFile f(ntuple_file, "recreate");
  framework::NtupleManager& n{framework::NtupleManager::getInstance()};
  REQUIRE_NOTHROW(n.create("test"));
  CHECK_THROWS(n.create("test"));

  REQUIRE_NOTHROW(n.addVar<bool>("test", "bool"));
  REQUIRE_NOTHROW(n.addVar<short>("test", "short"));
  REQUIRE_NOTHROW(n.addVar<int>("test", "int"));
  REQUIRE_NOTHROW(n.addVar<long>("test", "long"));
  REQUIRE_NOTHROW(n.addVar<float>("test", "float"));
  REQUIRE_NOTHROW(n.addVar<double>("test", "double"));
  REQUIRE_NOTHROW(n.addVar<std::vector<bool>>("test", "vector_bool"));
  REQUIRE_NOTHROW(n.addVar<std::vector<short>>("test", "vector_short"));
  REQUIRE_NOTHROW(n.addVar<std::vector<int>>("test", "vector_int"));
  REQUIRE_NOTHROW(n.addVar<std::vector<long>>("test", "vector_long"));
  REQUIRE_NOTHROW(n.addVar<std::vector<float>>("test", "vector_float"));
  REQUIRE_NOTHROW(n.addVar<std::vector<double>>("test", "vector_double"));

  CHECK_THROWS(n.addVar<float>("test", "float"));

  std::vector<bool> bools = {true, false, true};
  std::vector<short> shorts = {2, 3, 4};
  std::vector<int> ints = {2, 3, 4};
  std::vector<long> longs = {2, 3, 4};
  std::vector<float> floats = {0.2, 0.3, 0.4};
  std::vector<double> doubles = {0.2, 0.3, 0.4};

  std::vector<std::vector<bool>> vector_bools = {
      {true, true, true}, {false, false, false}, {true, false, false}};
  std::vector<std::vector<short>> vector_shorts = {
      {1, 2, 3}, {5, 7, 9}, {3, 4, 5}};
  std::vector<std::vector<int>> vector_ints = {{1, 2, 3}, {5, 7, 9}, {3, 4, 5}};
  std::vector<std::vector<long>> vector_longs = {
      {1, 2, 3}, {5, 7, 9}, {3, 4, 5}};
  std::vector<std::vector<float>> vector_floats = {
      {0.1, 0.01, 0.001}, {2e4, 3e5, 4e6}, {0.9, 0.99, 0.999}};
  std::vector<std::vector<double>> vector_doubles = {
      {0.1, 0.01, 0.001}, {2e4, 3e5, 4e6}, {0.9, 0.99, 0.999}};

  for (size_t i = 0; i < 3; i++) {
    // to save space, a std::vector<bool>
    //  compresses each bool into a one-bit
    //  type that can be converted back to the
    //  8-bit bool type.
    REQUIRE_NOTHROW(n.setVar("bool", bool(bools.at(i))));
    REQUIRE_NOTHROW(n.setVar("short", shorts.at(i)));
    REQUIRE_NOTHROW(n.setVar("int", ints.at(i)));
    REQUIRE_NOTHROW(n.setVar("long", longs.at(i)));
    REQUIRE_NOTHROW(n.setVar("float", floats.at(i)));
    REQUIRE_NOTHROW(n.setVar("double", doubles.at(i)));
    REQUIRE_NOTHROW(n.setVar("vector_bool", vector_bools.at(i)));
    REQUIRE_NOTHROW(n.setVar("vector_short", vector_shorts.at(i)));
    REQUIRE_NOTHROW(n.setVar("vector_int", vector_ints.at(i)));
    REQUIRE_NOTHROW(n.setVar("vector_long", vector_longs.at(i)));
    REQUIRE_NOTHROW(n.setVar("vector_float", vector_floats.at(i)));
    REQUIRE_NOTHROW(n.setVar("vector_double", vector_doubles.at(i)));
    CHECK_THROWS(n.setVar("bool", shorts.at(i)));
    CHECK_THROWS(n.setVar("bool", vector_bools.at(i)));
    n.fill();
    n.clear();
  }

  f.Write();
  f.Close();

  TTreeReader r("test", TFile::Open(ntuple_file));
  TTreeReaderValue<bool> root_bool(r, "bool");
  TTreeReaderValue<short> root_short(r, "short");
  TTreeReaderValue<int> root_int(r, "int");
  // root serializes 'long' (which on some systems is 64-bit)
  //  depending on the number of bits, so we need to read
  //  it in with 'long long' (which is at least 64-bit)
  //  to make sure we don't fail
  TTreeReaderValue<long long> root_long(r, "long");
  TTreeReaderValue<float> root_float(r, "float");
  TTreeReaderValue<double> root_double(r, "double");
  TTreeReaderValue<std::vector<bool>> root_vector_bool(r, "vector_bool");
  TTreeReaderValue<std::vector<short>> root_vector_short(r, "vector_short");
  TTreeReaderValue<std::vector<int>> root_vector_int(r, "vector_int");
  TTreeReaderValue<std::vector<long>> root_vector_long(r, "vector_long");
  TTreeReaderValue<std::vector<float>> root_vector_float(r, "vector_float");
  TTreeReaderValue<std::vector<double>> root_vector_double(r, "vector_double");

  for (size_t i = 0; i < 3; i++) {
    REQUIRE(r.Next());
    CHECK(*root_bool == bools.at(i));
    CHECK(*root_short == shorts.at(i));
    CHECK(*root_int == ints.at(i));
    CHECK(*root_long == longs.at(i));
    CHECK(*root_float == floats.at(i));
    CHECK(*root_double == doubles.at(i));
    // root takes the liberty to permut vectors when serializing
    //  in order to try to save space
    // This means we need to use unordered equals.
    // Not too much loss in efficiency becuase we keep our vectors short.
    CHECK_THAT(*root_vector_bool,
               Catch::Matchers::UnorderedEquals(vector_bools.at(i)));
    CHECK_THAT(*root_vector_short,
               Catch::Matchers::UnorderedEquals(vector_shorts.at(i)));
    CHECK_THAT(*root_vector_int,
               Catch::Matchers::UnorderedEquals(vector_ints.at(i)));
    CHECK_THAT(*root_vector_long,
               Catch::Matchers::UnorderedEquals(vector_longs.at(i)));
    CHECK_THAT(*root_vector_float,
               Catch::Matchers::UnorderedEquals(vector_floats.at(i)));
    CHECK_THAT(*root_vector_double,
               Catch::Matchers::UnorderedEquals(vector_doubles.at(i)));
  }

}  // process test
