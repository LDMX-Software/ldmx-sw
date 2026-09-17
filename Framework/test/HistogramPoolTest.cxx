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
#include "TFile.h"  //to open and check root files

using framework::HistogramPool;

/**
 * mock exception like the case where configuration did not provide a histogram
 * file
 */
TDirectory* cantCreateDir() {
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
    HistogramPool test_pool{cantCreateDir};
    // raise exception from get_directory
    REQUIRE_THROWS_WITH(test_pool.create("dne", "foo", 10, 0.0, 1.0),
                        "NOFILEGIVEN");
  }

  SECTION("creation on request") {
    TFile* histogram_file{nullptr};
    auto open_on_request = [&histogram_file, &test_file]() -> TDirectory* {
      if (histogram_file == nullptr) {
        histogram_file = TFile::Open(test_file, "recreate");
      }
      return histogram_file->mkdir("histo_directory");
    };
    HistogramPool test_pool{open_on_request};
    // test_file has not been opened yet
    REQUIRE(histogram_file == nullptr);
    test_pool.create("h", "bla", 10, 0, 1);
    // test_file exists with histo_directory/ inside of it
    REQUIRE(histogram_file != nullptr);
    CHECK(histogram_file->Get("histo_directory") != nullptr);
    histogram_file->Write();
    // test_file exists with histo_directory/h inside of it
    CHECK(histogram_file->Get("histo_directory/h") != nullptr);
  }

  SECTION("separate pools") {
    TFile histogram_file{test_file, "recreate"};
    HistogramPool test_pool_1{[&histogram_file]() {
      static TDirectory* d{histogram_file.mkdir("p1")};
      return d;
    }};
    HistogramPool test_pool_2{[&histogram_file]() {
      static TDirectory* d{histogram_file.mkdir("p2")};
      return d;
    }};
    CHECK(histogram_file.Get("p1") == nullptr);
    CHECK(histogram_file.Get("p2") == nullptr);
    test_pool_1.create("h", "bar", 10, 0, 1);
    CHECK(histogram_file.Get("p1") != nullptr);
    CHECK(histogram_file.Get("p2") == nullptr);
    test_pool_2.create("h", "buz", 10, 0, 10);
    CHECK(histogram_file.Get("p1") != nullptr);
    CHECK(histogram_file.Get("p2") != nullptr);
    histogram_file.Write();
    // test_file exists with
    //  p1/h -> 10 bins between 0 and 1
    //  p2/h -> 10 bins between 0 and 10
    auto h1 = dynamic_cast<TH1F*>(histogram_file.Get("p1/h"));
    auto h2 = dynamic_cast<TH1F*>(histogram_file.Get("p2/h"));
    REQUIRE(h1 != nullptr);
    REQUIRE(h2 != nullptr);
    CHECK(h1->GetNbinsX() == 10);
    CHECK(h2->GetNbinsX() == 10);
    CHECK(h1->GetBinLowEdge(10) == 0.9);
    CHECK(h2->GetBinLowEdge(10) == 9);
  }

  // different types of creation
  SECTION("different types of histograms") {
    TFile histogram_file{test_file, "recreate"};
    HistogramPool test_pool{[&histogram_file]() {
      static TDirectory* d{histogram_file.mkdir("p")};
      return d;
    }};
    std::vector<std::string> cats{"one", "two", "three"};

    test_pool.create("h1_1", "foo", 10, 0, 1);
    test_pool.create("h1_2", "bar", {0.0, 0.5, 0.8, 1.0}, true);
    test_pool.create("h1_3", "", cats);

    test_pool.create("h2_1", "baz", 5, -5, 5, "foo", 10, -5, 5);
    test_pool.create("h2_2", "baz", {-5.0, -1.0, 0.0, 1.0, 5.0}, "foo",
                     {0.0, 1.0, 5.0});
    test_pool.create("h2_3", "", cats, "", cats);

    test_pool.setWeight(0.75);
    test_pool.fill("h1_2", 0.75);
    test_pool.fillw("h1_2", 0.1, 0.5);
    test_pool.fill("h1_2", 0.75);
    test_pool.setWeight(1);

    test_pool.fill("h1_3", "one");
    // "one" again because categories are zero-indexed into bins
    test_pool.fill("h1_3", 0);
    // "two" because categories are zero-indexed into bins
    test_pool.fill("h1_3", 1);
    // "three" into third bin
    test_pool.fill("h1_3", "three");
    // unknown category auto-expands Nbins when
    // doing categorical axes
    // test_pool.fill("h1_3", "four");

    test_pool.fill("h2_3", "one", "two");

    histogram_file.Write();

    auto h1_1 = dynamic_cast<TH1F*>(histogram_file.Get("p/h1_1"));
    REQUIRE(h1_1 != nullptr);
    CHECK(h1_1->GetNbinsX() == 10);
    CHECK(h1_1->GetBinLowEdge(1) == 0.0);
    CHECK(h1_1->GetBinLowEdge(11) == 1.0);
    REQUIRE(h1_1->GetSumw2() != nullptr);
    CHECK(h1_1->GetSumw2()->fN == 0);

    auto h1_2 = dynamic_cast<TH1F*>(histogram_file.Get("p/h1_2"));
    REQUIRE(h1_2 != nullptr);
    CHECK(h1_2->GetNbinsX() == 3);
    CHECK(h1_2->GetBinLowEdge(1) == 0.0);
    CHECK(h1_2->GetBinLowEdge(2) == 0.5);
    CHECK(h1_2->GetBinLowEdge(3) == 0.8);
    CHECK(h1_2->GetBinLowEdge(4) == 1.0);
    CHECK(h1_2->GetBinContent(1) == 0.5);
    CHECK(h1_2->GetBinContent(2) == 2 * 0.75);
    REQUIRE(h1_2->GetSumw2() != nullptr);
    CHECK(h1_2->GetSumw2()->fN == 5);
    CHECK(h1_2->GetSumw2()->At(1) == 0.25);
    CHECK(h1_2->GetSumw2()->At(2) == 0.75 * 0.75 * 2);

    auto h1_3 = dynamic_cast<TH1F*>(histogram_file.Get("p/h1_3"));
    REQUIRE(h1_3 != nullptr);
    CHECK(h1_3->GetNbinsX() == 3);

    CHECK(h1_3->GetBinContent(0) == 0);  // under
    CHECK(h1_3->GetBinContent(1) == 2);  // one
    CHECK(h1_3->GetBinContent(2) == 1);  // two
    CHECK(h1_3->GetBinContent(3) == 1);  // three
    CHECK(h1_3->GetBinContent(4) == 0);  // over

    auto h2_1 = dynamic_cast<TH2F*>(histogram_file.Get("p/h2_1"));
    REQUIRE(h2_1 != nullptr);
    CHECK(h2_1->GetNbinsX() == 5);
    CHECK(h2_1->GetNbinsY() == 10);

    auto h2_2 = dynamic_cast<TH2F*>(histogram_file.Get("p/h2_2"));
    REQUIRE(h2_2 != nullptr);
    CHECK(h2_2->GetNbinsX() == 4);
    CHECK(h2_2->GetNbinsY() == 2);

    auto h2_3 = dynamic_cast<TH2F*>(histogram_file.Get("p/h2_3"));
    REQUIRE(h2_3 != nullptr);
    CHECK(h2_3->GetBinContent(1, 2) == 1);
  }

  SECTION("scale with electron count") {
    TFile histogram_file{test_file, "recreate"};
    HistogramPool test_pool{[&histogram_file]() {
      static TDirectory* d{histogram_file.mkdir("s")};
      return d;
    }};
    std::vector<double> edges{0.0, 2.5, 5.0, 7.5, 10.0};
    test_pool.create("one_e", "", edges);
    test_pool.create("fixed", "", 4, 0, 10);
    test_pool.create("vari", "", {0.0, 1.0, 10.0});
    test_pool.create("h2", "", 4, 0, 10, "", 4, 0, 10);
    test_pool.scaleWithElectrons("one_e");
    CHECK_THROWS(test_pool.scaleWithElectrons("vari"));
    CHECK_THROWS(test_pool.scaleWithElectrons("h2"));
    test_pool.fill("fixed", 1.);
    CHECK_THROWS(test_pool.scaleWithElectrons("fixed"));

    // widened while filling
    CHECK(test_pool.get("one_e")->GetXaxis()->GetXmax() ==
          Catch::Approx(10. * HistogramPool::MAX_ELECTRON_SCALE));

    // one electron events only, over range goes to overflow
    for (double v : {1., 9., 15., 25., 1000.}) test_pool.fill("one_e", v);
    test_pool.setElectronCount(0);
    test_pool.setElectronCount(1);
    test_pool.trimToElectronCount();
    auto one_e = test_pool.get("one_e");
    CHECK(one_e->GetNbinsX() == 4);
    CHECK(one_e->GetXaxis()->GetXmax() == Catch::Approx(10.));
    CHECK(one_e->GetBinContent(1) == 1);
    CHECK(one_e->GetBinContent(4) == 1);
    CHECK(one_e->GetBinContent(5) == 3);
    CHECK(one_e->GetEntries() == 5);
    CHECK(one_e->GetMean() == Catch::Approx(5.));

    // a two electron event doubles the range
    HistogramPool pool_2e{[&histogram_file]() {
      static TDirectory* d{histogram_file.mkdir("s2")};
      return d;
    }};
    pool_2e.create("h", "", 4, 0, 10, true);
    pool_2e.scaleWithElectrons("h");
    pool_2e.setElectronCount(1);
    pool_2e.fillw("h", 1., 2.);
    pool_2e.fillw("h", 18., 2.);
    pool_2e.fillw("h", 25., 2.);
    pool_2e.setElectronCount(2);
    pool_2e.setElectronCount(1);
    pool_2e.trimToElectronCount();
    auto two_e = pool_2e.get("h");
    CHECK(two_e->GetNbinsX() == 8);
    CHECK(two_e->GetXaxis()->GetXmax() == Catch::Approx(20.));
    CHECK(two_e->GetBinWidth(1) == Catch::Approx(2.5));
    CHECK(two_e->GetBinContent(1) == 2);
    CHECK(two_e->GetBinContent(8) == 2);
    CHECK(two_e->GetBinContent(9) == 2);
    CHECK(two_e->GetBinError(9) == Catch::Approx(2.));
    CHECK(two_e->GetEntries() == 3);

    // counts past the cap are clamped
    HistogramPool pool_many{[&histogram_file]() {
      static TDirectory* d{histogram_file.mkdir("s3")};
      return d;
    }};
    pool_many.create("h", "", 4, 0, 10);
    pool_many.scaleWithElectrons("h");
    pool_many.setElectronCount(100);
    pool_many.trimToElectronCount();
    CHECK(pool_many.get("h")->GetNbinsX() ==
          4 * HistogramPool::MAX_ELECTRON_SCALE);
  }
}
