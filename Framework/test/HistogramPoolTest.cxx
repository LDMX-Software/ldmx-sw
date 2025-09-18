/**
 * @file HistogramPoolTest.cxx
 * @brief Test the operation of Framework processing
 *
 * @author Tom Eichlersmith, University of Minnesota
 *
 */
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "Framework/HistogramPool.h"
#include "TFile.h"        //to open and check root files

using framework::HistogramPool;

/**
 * mock exception like the case where configuration did not provide a histogram file
 */
TDirectory* cant_create_dir() {
  throw std::runtime_error("NOFILEGIVEN");
  return nullptr;
}

/**
 * Test for HistogramPool
 *
 * We check that the HistogramPool can successfully create
 * the different types of histograms and keep them separate
 * from other pool's histograms even if they have the same
 * name.
 */
TEST_CASE("HistogramPool Functions", "[Framework][functionality]") {
  const char* test_file = "/tmp/test_histogram_pool.root";

  SECTION("exception on request") {
    HistogramPool p{cant_create_dir};
    // raise exception from get_directory
    REQUIRE_THROWS_WITH(
        p.create("dne", "foo", 10, 0.0, 1.0),
        "NOFILEGIVEN"
    );
  }

  SECTION("creation on request") {
    TFile* f{nullptr};
    auto open_on_request = [&f, &test_file]() -> TDirectory* {
      if (f == nullptr) {
        f = TFile::Open(test_file, "recreate");
      }
      return f->mkdir("histo_directory");
    };
    HistogramPool p{open_on_request};
    // test_file has not been opened yet
    REQUIRE(f == nullptr);
    p.create("h", "bla", 10, 0, 1);
    // test_file exists with histo_directory/ inside of it
    REQUIRE(f != nullptr);
    CHECK(f->Get("histo_directory") != nullptr);
    f->Write();
    // test_file exists with histo_directory/h inside of it
    CHECK(f->Get("histo_directory/h") != nullptr);
  }

  SECTION("separate pools") {
    TFile f{test_file, "recreate"};
    HistogramPool p1{[&f]() { return f.mkdir("p1"); }};
    HistogramPool p2{[&f]() { return f.mkdir("p2"); }};
    CHECK(f.Get("p1") == nullptr);
    CHECK(f.Get("p2") == nullptr);
    p1.create("h", "bar", 10, 0, 1);
    CHECK(f.Get("p1") != nullptr);
    CHECK(f.Get("p2") == nullptr);
    p2.create("h", "buz", 10, 0, 10);
    CHECK(f.Get("p1") != nullptr);
    CHECK(f.Get("p2") != nullptr);
    f.Write();
    // test_file exists with
    //  p1/h -> 10 bins between 0 and 1
    //  p2/h -> 10 bins between 0 and 10
    auto h1 = dynamic_cast<TH1F*>(f.Get("p1/h"));
    auto h2 = dynamic_cast<TH1F*>(f.Get("p2/h"));
    REQUIRE(h1 != nullptr);
    REQUIRE(h2 != nullptr);
    CHECK(h1->GetNbinsX() == 10);
    CHECK(h2->GetNbinsX() == 10);
    CHECK(h1->GetBinLowEdge(10) == 0.9);
    CHECK(h2->GetBinLowEdge(10) == 9);
  }

  // different types of creation
}
