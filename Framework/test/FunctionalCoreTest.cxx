#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <cstdio>  //for remove

#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "Framework/RunHeader.h"
#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalVetoResult.h"
#include "Recon/Event/CalorimeterHit.h"
#include "TFile.h"        //to open and check root files
#include "TH1F.h"         //for test histogram
#include "TTreeReader.h"  //to check output event files

using Catch::Approx;

namespace framework {
namespace test {

/**
 * @class TestProducer
 * Bare producer that creates a collection and an object and puts them
 * on the event bus.
 *
 * The pattern this producer creates is the following:
 * - The vector of Calorimeter hits_ has the same number of entries as the event
 * number
 * - The IDs of the calorimeter hits_ are set to 10*eventNumber+their_index
 * - The input object is an HcalVetoResult where events with an event index_
 * pass
 * - The max PE hit in the HcalVetoResult has an ID equal to the event index_
 * - If a run header is created, the event count and the run number are equal
 *
 * Checks
 * - Event::add function does not throw any errors.
 * - Writes and adds a run header where the run number and the number of events
 * are the same.
 * - sets a storage hint
 */
class TestProducer : public Producer {
  /// number of events we've gotten to
  int events_;

  /// should we create the run header?
  bool create_run_header_;

 public:
  TestProducer(const std::string& name, Process& p) : Producer(name, p) {}
  ~TestProducer() {}

  void configure(framework::config::Parameters& p) final override {
    create_run_header_ = p.get<bool>("create_run_header");
  }

  void beforeNewRun(ldmx::RunHeader& header) final override {
    if (not create_run_header_) return;
    header.setIntParameter("Should Be Run Number", header.getRunNumber());
    return;
  }

  void produce(framework::Event& event) final override {
    int i_event = event.getEventNumber();

    REQUIRE(i_event > 0);

    std::vector<ldmx::CalorimeterHit> calo_hits;
    for (int i = 0; i < i_event; i++) {
      calo_hits.emplace_back();
      calo_hits.back().setID(i_event * 10 + i);
    }

    REQUIRE_NOTHROW(event.add("TestCollection", calo_hits));

    ldmx::HcalHit max_pe_hit;
    max_pe_hit.setID(i_event);

    ldmx::HcalVetoResult res;
    res.setMaxPEHit(max_pe_hit);
    res.setVetoResult(i_event % 2 == 0);

    REQUIRE_NOTHROW(event.add("TestObject", res));

    events_ = i_event;

    std::vector<int> event_indices = {i_event, i_event};
    REQUIRE_NOTHROW(event.add("EventIndex", event_indices));

    float test_float = i_event * 0.1;
    REQUIRE_NOTHROW(event.add("EventTenth", test_float));

    if (res.passesVeto()) setStorageHint(StorageControl::Hint::MustKeep);

    return;
  }
};  // TestProducer

/**
 * @class TestAnalyzer
 * Bare analyzer that looks for objects matching what the TestProducer put in.
 *
 * Checks
 * - the correct number and contents following the pattern produced by
 * TestProducer.
 * - Event::getCollection and Event::getObject don't throw errors.
 */
class TestAnalyzer : public Analyzer {
 private:
  std::string test_collection_passname_;
  std::string test_object_passname_;
  std::string veto_test_object_passname_;
  std::string tenth_event_passname_;
  std::string event_index_passname_;

 public:
  TestAnalyzer(const std::string& name, Process& p) : Analyzer(name, p) {}
  ~TestAnalyzer() {}

  void configure(framework::config::Parameters& ps) override {
    test_collection_passname_ =
        ps.get<std::string>("test_collection_passname", "");
    test_object_passname_ = ps.get<std::string>("test_object_passname", "");
    veto_test_object_passname_ =
        ps.get<std::string>("veto_test_object_passname", "");
    tenth_event_passname_ = ps.get<std::string>("tenth_event_passname", "");
    event_index_passname_ = ps.get<std::string>("event_index_passname", "");
  }

  void onProcessStart() final override {
    REQUIRE_NOTHROW(getHistoDirectory());
    test_hist_ = new TH1F("test_hist_", "Test Histogram", 101, -50, 50);
    test_hist_->SetCanExtend(TH1::kAllAxes);
  }

  void analyze(const framework::Event& event) final override {
    int i_event = event.getEventNumber();

    REQUIRE(i_event > 0);

    const std::vector<ldmx::CalorimeterHit>& calo_hits =
        event.getCollection<ldmx::CalorimeterHit>("TestCollection",
                                                  test_collection_passname_);

    CHECK(calo_hits.size() == i_event);
    for (unsigned int i = 0; i < calo_hits.size(); i++) {
      CHECK(calo_hits.at(i).getID() == i_event * 10 + i);
      test_hist_->Fill(calo_hits.at(i).getID());
    }

    const ldmx::HcalVetoResult& veto_res =
        event.getObject<ldmx::HcalVetoResult>("TestObject",
                                              veto_test_object_passname_);

    auto max_pe_hit{veto_res.getMaxPEHit()};

    CHECK(max_pe_hit.getID() == i_event);
    CHECK(veto_res.passesVeto() == (i_event % 2 == 0));

    const float& tenth_event =
        event.getObject<float>("EventTenth", tenth_event_passname_);
    CHECK(tenth_event == Approx(i_event * 0.1));

    const std::vector<int>& i_event_from_bus =
        event.getCollection<int>("EventIndex", event_index_passname_);

    CHECK(i_event_from_bus.size() == 2);
    CHECK(i_event_from_bus.at(0) == i_event);
    CHECK(i_event_from_bus.at(1) == i_event);

    return;
  }

 private:
  /// test histogram filled with event indices
  TH1F* test_hist_;
};  // TestAnalyzer

/**
 * @class isGoodHistogramFile
 *
 * Runs a variety of checks to make sure the
 * histogram in the input filename is what we expect it to be.
 *
 * Does NOT check that the entries in the histogram are correct,
 * just makes sure that there is the correct number.
 *
 * Does check:
 * - The input file name is readable
 * - The input file has the directory "TestAnalyzer"
 * - The directory has the histogram "test_hist_"
 * - The histogram has the correct number of entries
 */
class IsGoodHistogramFile : public Catch::Matchers::MatcherBase<std::string> {
 private:
  /// Correct number of entries
  int correct_get_entries_;

 public:
  /**
   * Constructor
   *
   * Sets the correct event indices
   */
  IsGoodHistogramFile(int const& n) : correct_get_entries_(n) {}

  /**
   * Performs the test for this matcher
   *
   * Opens up the histogram file and makes sure of a few things.
   * - The histogram 'test_hist_' is in the 'TestAnalyzer' directory
   * - The histogram has the correct number of entries
   */
  bool match(const std::string& filename) const override {
    // Open file
    TFile* f = TFile::Open(filename.c_str());
    if (!f) return false;
    TDirectory* d = (TDirectory*)f->Get("TestAnalyzer");
    if (!d) return false;
    TH1F* h = (TH1F*)d->Get("test_hist_");
    if (!h) return false;

    return (h->GetEntries() == correct_get_entries_);
  }

  /**
   * Describe this matcher in a helpful, human-readable way.
   *
   * This string is written as if stating a fact about
   * the object it is matching.
   */
  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << "has the histogram 'TestAnalyzer/test_hist_' with the number of "
          "entries "
       << correct_get_entries_;
    return ss.str();
  }
};  // isGoodHistogramFile

/**
 * @class isGoodEventFile
 *
 * Checks:
 * - Event File exists and is readable
 * - Has the LDMX_Events TTree
 * - Events tree has correct number of entries
 * - if existCollection: looks through collections on the event tree to make
 * sure they have the same form as set by TestProducer
 * - if existObject: looks through objects on event tree to make sure they have
 * the same form as set by TestProducer
 * - Has the LDMX_Run TTree
 * - Run tree has correct number of entries
 * - RunHeaders in RunTree have matching RunNumbers and EventCounts
 */
class IsGoodEventFile : public Catch::Matchers::MatcherBase<std::string> {
 private:
  /// pass name to check the collection and/or object for
  std::string pass_;

  /// correct number of entries in the event ttree
  int entries_;

  /// correct number of runs
  int runs_;

  /// collection should exist in file
  bool exist_collection_;

  /// object should exist in file
  bool exist_object_;

 public:
  /**
   * Constructor
   *
   * Sets the correct number of entries and the other checking parameters
   */
  IsGoodEventFile(const std::string& pass, const int& entries, const int& runs,
                  bool existColl = true, bool existObj = true)
      : pass_(pass),
        entries_(entries),
        runs_(runs),
        exist_collection_(existColl),
        exist_object_(existObj) {}

  /**
   * Actually do the matching
   *
   * The event and run tree names are hardcoded.
   * The branchnames are also hardcoded.
   *
   * @param[in] filename name of event file to check
   */
  bool match(const std::string& filename) const override {
    TFile* f = TFile::Open(filename.c_str());
    if (!f) return false;

    TTreeReader events("LDMX_Events", f);

    if (events.GetEntries(true) != entries_) {
      f->Close();
      return false;
    }

    // Event tree should _always_ have the ldmx::EventHeader
    TTreeReaderValue<ldmx::EventHeader> header(events, "EventHeader");

    if (exist_collection_) {
      // make sure collection matches pattern
      TTreeReaderValue<std::vector<ldmx::CalorimeterHit>> collection(
          events, ("test_collection_" + pass_).c_str());
      while (events.Next()) {
        if (collection->size() != header->getEventNumber()) {
          f->Close();
          return false;
        }
        for (unsigned int i = 0; i < collection->size(); i++)
          if (collection->at(i).getID() != header->getEventNumber() * 10 + i) {
            f->Close();
            return false;
          }
      }
      // restart in case checking object as well
      events.Restart();
    } else {
      // check to make sure collection is NOT there
      auto t{(TTree*)f->Get("LDMX_Events")};
      if (t and t->GetBranch(("test_collection_" + pass_).c_str())) {
        f->Close();
        return false;
      }
    }

    if (exist_object_) {
      // make sure object matches pattern
      TTreeReaderValue<ldmx::HcalVetoResult> object(
          events, ("test_object_" + pass_).c_str());
      while (events.Next()) {
        if (object->getMaxPEHit().getID() != header->getEventNumber()) {
          f->Close();
          return false;
        }
      }
    } else {
      // check to make sure object is NOT there
      auto t{(TTree*)f->Get("LDMX_Events")};
      if (t and t->GetBranch(("test_object_" + pass_).c_str())) {
        f->Close();
        return false;
      }
    }

    TTreeReader runs("LDMX_Run", f);

    if (runs.GetEntries(true) != runs_) {
      f->Close();
      return false;
    }

    TTreeReaderValue<ldmx::RunHeader> run_header(runs, "RunHeader");

    while (runs.Next()) {
      if (run_header->getRunNumber() !=
          run_header->getIntParameter("Should Be Run Number")) {
        f->Close();
        return false;
      }
    }

    f->Close();

    return true;
  }

  /**
   * Human-readable statement for any match that is true.
   */
  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << "can be opened and has the correct number of entries in the event "
          "tree and the run tree.";

    ss << " test_collection_" << pass_ << " was verified to ";
    if (exist_collection_)
      ss << " be the correct pattern.";
    else
      ss << " not be in the file.";

    ss << " test_object_" << pass_ << " was verified to ";
    if (exist_object_)
      ss << " be the correct pattern.";
    else
      ss << " not be in the file.";

    return ss.str();
  }

};  // isGoodEventFile

/**
 * @func removeFile
 * Deletes the file and returns whether the deletion was successful.
 *
 * This is just a helper function during development.
 * Sometimes it is helpful to leave the generated files, so
 * maybe we can make the removal optional?
 */
static bool removeFile(const std::string& filepath) {
  return remove(filepath.c_str()) == 0;
}

/**
 * @func run the process for the input parameters
 */
static bool runProcess(const framework::config::Parameters& parameters) {
  ProcessHandle p;
  try {
    p = std::make_unique<Process>(parameters);
  } catch (framework::exception::Exception& e) {
    std::cerr << "Config Error [" << e.name() << "] : " << e.message()
              << std::endl;
    std::cerr << "  at " << e.module() << ":" << e.line() << " in "
              << e.function() << std::endl;
    return false;
  }
  p->run();
  return true;
}

}  // namespace test
}  // namespace framework

DECLARE_PRODUCER(framework::test::TestProducer)
DECLARE_ANALYZER(framework::test::TestAnalyzer)

/**
 * Test for C++ Framework processing.
 *
 * This test is aimed at checking that core functionalities are operational.
 * Python configuration, Simulation, and other add-on functionalities
 * are tested separately.
 *
 * This test does not check complicated combinations for drop/keep rules and
 * skimming rules. I (Tom E) avoided this because this test is already very
 * large and complicated.
 * TODO write a more full (and separate) test for these parts of the framework.
 *
 * Assumptions:
 *  - Any vector of objects behaves like a vector of CalorimeterHits when viewed
 * from core
 *  - Any object behaves like a ldmx::HcalVetoResult when viewed from core
 *
 * What does this even test?
 *  - Event::add an object and a vector of objects (changing size and content)
 *  - Event::get an object and a vector of objects (changing size and content)
 *  - Event can switch to different input tree (Multiple Input Files)
 *  - Creating and filling a histogram
 *  - Reading from input file(s)
 *  - Production Mode (no input files)
 *  - Analysis Mode (no output event files, only output histogram file)
 *  - Merge Mode (several input files to one output file)
 *  - N-to-N Mode (several input files to several output files)
 *  - writing histogram to a file
 *  - writing to an output file
 *  - writing and reading run headers
 *  - drop/keep rules for event bus passengers
 *  - skimming events (only keeping events meeting a certain criteria)
 */
TEST_CASE("Core Framework Functionality", "[Framework][functionality]") {
  // these parameters aren't tested/changed, so we set them out here
  framework::config::Parameters process;
  process.add("compressionSetting", 9);
  process.add("maxTriesPerEvent", 1);
  process.add("logFrequency", -1);
  process.add("termLogLevel", 4);
  process.add("fileLogLevel", 4);
  process.add<std::string>("logFileName", "");
  process.add<std::string>("tree_name", "LDMX_Events");

  framework::config::Parameters producer_parameters;
  producer_parameters.add<std::string>("class_name",
                                       "framework::test::TestProducer");
  producer_parameters.add<std::string>("instanceName", "TestProducer");

  framework::config::Parameters analyzer_parameters;
  analyzer_parameters.add<std::string>("class_name",
                                       "framework::test::TestAnalyzer");
  analyzer_parameters.add<std::string>("instanceName", "TestAnalyzer");

  // declare used and re-used types, not used in all branches
  std::vector<framework::config::Parameters> sequence;
  std::vector<std::string> input_files, output_files;

  SECTION("Production Mode") {
    // no input files, only output files

    output_files = {"test_productionmode_events.root"};
    process.add<std::string>("pass_name", "test");
    process.add("outputFiles", output_files);
    process.add("maxEvents", 3);
    process.add("run", 3);

    producer_parameters.add("create_run_header", true);

    sequence = {producer_parameters};

    SECTION("only producers") {
      process.add("sequence", sequence);
      process.add<std::string>("histogramFile", "");
      SECTION("no drop/keep rules") {
        // Process owns and deletes the processors
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(output_files.at(0),
                   framework::test::IsGoodEventFile("test", 3, 1));
      }

      SECTION("drop TestCollection") {
        std::vector<std::string> keep = {"drop .*Collection.*"};
        process.add("keep", keep);
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(output_files.at(0),
                   framework::test::IsGoodEventFile("test", 3, 1, false));
      }

      SECTION("skim for even indexed events") {
        process.add("skimDefaultIsKeep", false);
        std::vector<std::string> rules = {"TestProducer", ""};
        process.add("skimRules", rules);
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(output_files.at(0),
                   framework::test::IsGoodEventFile("test", 1, 1));
      }
    }

    SECTION("with Analyses") {
      std::string hist_file_path =
          "test_productionmode_withanalyses_hists.root";

      process.add("histogramFile", hist_file_path);

      sequence.push_back(analyzer_parameters);
      process.add("sequence", sequence);

      SECTION("no drop/keep rules") {
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(output_files.at(0),
                   framework::test::IsGoodEventFile("test", 3, 1));
      }

      SECTION("drop TestCollection") {
        std::vector<std::string> keep = {"drop .*Collection.*"};
        process.add("keep", keep);
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(output_files.at(0),
                   framework::test::IsGoodEventFile("test", 3, 1, false));
      }

      SECTION("skim for even indexed events") {
        process.add("skimDefaultIsKeep", false);
        std::vector<std::string> rules = {"TestProducer", ""};
        process.add("skimRules", rules);
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(output_files.at(0),
                   framework::test::IsGoodEventFile("test", 1, 1));
      }

      CHECK_THAT(hist_file_path,
                 framework::test::IsGoodHistogramFile(1 + 2 + 3));
      CHECK(framework::test::removeFile(hist_file_path));
    }

    CHECK(framework::test::removeFile(output_files.at(0)));
  }  // Production Mode

  SECTION("Need Input Files") {
    input_files = {"test_needinputfiles_2_events.root",
                   "test_needinputfiles_3_events.root",
                   "test_needinputfiles_4_events.root"};

    for (int run{2}; run < 5; run++) {
      auto make_inputs = process;
      make_inputs.add<std::string>("pass_name", "makeInputs");
      auto producer = producer_parameters;
      producer.add("create_run_header", true);
      make_inputs.add<std::vector<framework::config::Parameters>>("sequence",
                                                                  {producer});
      output_files = {input_files.at(run - 2)};
      make_inputs.add("outputFiles", output_files);
      make_inputs.add("maxEvents", run);
      make_inputs.add("run", run);
      REQUIRE(framework::test::runProcess(make_inputs));
      REQUIRE_THAT(input_files.at(run - 2),
                   framework::test::IsGoodEventFile("makeInputs", run, 1));
    }
    process.add<std::string>("pass_name", "test");

    SECTION("Analysis Mode") {
      // no output files, only histogram output

      sequence = {analyzer_parameters};
      process.add("sequence", sequence);

      std::string hist_file_path = "test_analysismode_hists.root";
      process.add("histogramFile", hist_file_path);

      SECTION("one input file") {
        std::vector<std::string> input_file = {input_files.at(0)};
        process.add("inputFiles", input_file);
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(hist_file_path, framework::test::IsGoodHistogramFile(1 + 2));
        CHECK(framework::test::removeFile(hist_file_path));
      }

      SECTION("multiple input files") {
        process.add("inputFiles", input_files);
        REQUIRE(framework::test::runProcess(process));
        CHECK_THAT(hist_file_path, framework::test::IsGoodHistogramFile(
                                       1 + 2 + 1 + 2 + 3 + 1 + 2 + 3 + 4));
        CHECK(framework::test::removeFile(hist_file_path));
      }

    }  // Analysis Mode

    SECTION("Merge Mode") {
      // many input files to one output file

      process.add("inputFiles", input_files);

      std::string event_file_path = "test_mergemode_events.root";
      output_files = {event_file_path};
      process.add("outputFiles", output_files);

      SECTION("with analyzers") {
        sequence = {analyzer_parameters};

        std::string hist_file_path = "test_mergemode_withanalyzers_hists.root";

        process.add("sequence", sequence);
        process.add("histogramFile", hist_file_path);

        SECTION("no drop/keep rules") {
          REQUIRE(framework::test::runProcess(process));
          CHECK_THAT(event_file_path, framework::test::IsGoodEventFile(
                                          "makeInputs", 2 + 3 + 4, 3));
        }

        SECTION("drop TestCollection") {
          std::vector<std::string> keep = {"drop .*Collection.*"};
          process.add("keep", keep);
          REQUIRE(framework::test::runProcess(process));
          CHECK_THAT(event_file_path, framework::test::IsGoodEventFile(
                                          "makeInputs", 2 + 3 + 4, 3, false));
        }

        CHECK_THAT(hist_file_path, framework::test::IsGoodHistogramFile(
                                       1 + 2 + 1 + 2 + 3 + 1 + 2 + 3 + 4));
        CHECK(framework::test::removeFile(hist_file_path));
      }

      SECTION("with producers") {
        producer_parameters.add("create_run_header", false);
        sequence = {producer_parameters};

        process.add("sequence", sequence);

        SECTION("not listening to storage hints") {
          REQUIRE(framework::test::runProcess(process));
          CHECK_THAT(event_file_path,
                     framework::test::IsGoodEventFile("test", 2 + 3 + 4, 3));
        }

        SECTION("skim for even indexed events") {
          process.add("skimDefaultIsKeep", false);
          std::vector<std::string> rules = {"TestProducer", ""};
          process.add("skimRules", rules);
          REQUIRE(framework::test::runProcess(process));
          CHECK_THAT(event_file_path,
                     framework::test::IsGoodEventFile("test", 1 + 1 + 2, 3));
        }
      }

      CHECK(framework::test::removeFile(event_file_path));

    }  // Merge Mode

  }  // need input files

}  // process test
