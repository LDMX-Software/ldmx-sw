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
    HistogramPool p1{[&f]() { 
      static TDirectory *d{f.mkdir("p1")};
      return d;
    }};
    HistogramPool p2{[&f]() {
      static TDirectory *d{f.mkdir("p2")};
      return d;
    }};
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
  SECTION("different types of histograms") {
    TFile f{test_file, "recreate"};
    HistogramPool p{[&f]() {
      static TDirectory* d{f.mkdir("p")};
      return d;
    }};
    std::vector<std::string> cats{"one", "two", "three"};

    p.create("h1_1", "foo", 10, 0, 1);
    p.create("h1_2", "bar", {0.0, 0.5, 0.8, 1.0});
    p.create("h1_3", cats);

    p.create("h2_1", "baz", 5, -5, 5, "foo", 10, -5, 5);
    p.create("h2_2", "baz", {-5.0, -1.0, 0.0, 1.0, 5.0}, "foo", {0.0, 1.0, 5.0});

    p.fill("h1_3", 1);
//    p.fill("h1_3", "one");
//    p.fill("h1_3", "three");
//    p.fill("h1_3", "four");
    p.fill("h1_3", 0);

    f.Write();

    auto h1_1 = dynamic_cast<TH1F*>(f.Get("p/h1_1"));
    REQUIRE(h1_1 != nullptr);
    CHECK(h1_1->GetNbinsX() == 10);
    CHECK(h1_1->GetBinLowEdge(1) == 0.0);
    CHECK(h1_1->GetBinLowEdge(11) == 1.0);

    auto h1_2 = dynamic_cast<TH1F*>(f.Get("p/h1_2"));
    REQUIRE(h1_2 != nullptr);
    CHECK(h1_2->GetNbinsX() == 3);
    CHECK(h1_2->GetBinLowEdge(1) == 0.0);
    CHECK(h1_2->GetBinLowEdge(2) == 0.5);
    CHECK(h1_2->GetBinLowEdge(3) == 0.8);
    CHECK(h1_2->GetBinLowEdge(4) == 1.0);

    auto h1_3 = dynamic_cast<TH1F*>(f.Get("p/h1_3"));
    REQUIRE(h1_3 != nullptr);
    CHECK(h1_3->GetNbinsX() == 3);

    CHECK(h1_3->GetBinContent(0) == 1);
    CHECK(h1_3->GetBinContent(1) == 2);
    CHECK(h1_3->GetBinContent(2) == 0);
    CHECK(h1_3->GetBinContent(3) == 0);
    CHECK(h1_3->GetBinContent(4) == 1);


    auto h2_1 = dynamic_cast<TH2F*>(f.Get("p/h2_1"));
    REQUIRE(h2_1 != nullptr);
    CHECK(h2_1->GetNbinsX() == 5);
    CHECK(h2_1->GetNbinsY() == 10);

    auto h2_2 = dynamic_cast<TH2F*>(f.Get("p/h2_2"));
    REQUIRE(h2_2 != nullptr);
    CHECK(h2_2->GetNbinsX() == 4);
    CHECK(h2_2->GetNbinsY() == 2);
  }
}
