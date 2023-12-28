
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <fstream>  // ifstream, ofstream

#include "Framework/ConfigurePython.h"
#include "Framework/EventProcessor.h"
#include "Framework/Process.h"
#include "Python.h"

using Catch::Approx;
using Catch::Matchers::ContainsSubstring;

namespace framework {
namespace test {

/**
 * @class TestConfig
 * @brief Defines a test Producer to test the passing of configuration
 * variables
 */
class TestConfig : public framework::Producer {
 public:
  /**
   * Constructor
   *
   * Follows the standard form for a framework::Producer.
   *
   * Checks that the passed name is the same as
   * what is written to the python config script.
   */
  TestConfig(const std::string &name, framework::Process &process)
      : framework::Producer(name, process) {
    CHECK(name == "test_instance");
  }

  /**
   * Configure function
   *
   * Checks:
   * - int parameter
   * - double parameter
   * - string parameter
   * - dictionary parameter
   * - vector of ints parameter
   * - vector of doubles parameter
   * - vector of strings parameter
   */
  void configure(framework::config::Parameters &parameters) final override {
    // Check parameters
    CHECK(parameters.getParameter<int>("test_int") == 9);
    CHECK(parameters.getParameter<double>("test_double") == Approx(7.7));
    CHECK(parameters.getParameter<std::string>("test_string") == "Yay!");

    // Check dictionary
    auto test_dict{
        parameters.getParameter<framework::config::Parameters>("test_dict")};
    CHECK(test_dict.getParameter<int>("one") == 1);
    CHECK(test_dict.getParameter<double>("two") == 2.0);

    // Check int vector
    std::vector<int> int_vect{1, 2, 3};
    auto test_int_vec{
        parameters.getParameter<std::vector<int>>("test_int_vec")};
    REQUIRE(test_int_vec.size() == int_vect.size());
    for (std::size_t i{0}; i < test_int_vec.size(); i++)
      CHECK(test_int_vec.at(i) == int_vect.at(i));

    // Check double vec
    std::vector<double> double_vec{0.1, 0.2, 0.3};
    auto test_double_vec{
        parameters.getParameter<std::vector<double>>("test_double_vec")};
    REQUIRE(test_double_vec.size() == double_vec.size());
    for (std::size_t i{0}; i < test_double_vec.size(); i++)
      CHECK(test_double_vec.at(i) == double_vec.at(i));

    // Check string vector
    std::vector<std::string> string_vec{"first", "second", "third"};
    auto test_string_vec{
        parameters.getParameter<std::vector<std::string>>("test_string_vec")};
    REQUIRE(test_string_vec.size() == string_vec.size());
    for (std::size_t i{0}; i < test_string_vec.size(); i++)
      CHECK(test_string_vec.at(i) == string_vec.at(i));

    // check 2d vector
    std::vector<std::vector<int>> twod_vec{
        {11, 12, 13}, {21, 22}, {31, 32, 33, 34}};
    auto test_2d_vec{
        parameters.getParameter<std::vector<std::vector<int>>>("test_2dlist")};
    REQUIRE(test_2d_vec.size() == twod_vec.size());
    for (std::size_t i{0}; i < twod_vec.size(); i++) {
      REQUIRE(test_2d_vec.at(i).size() == twod_vec.at(i).size());
      for (std::size_t j{0}; j < twod_vec.at(i).size(); j++) {
        CHECK(test_2d_vec.at(i).at(j) == twod_vec.at(i).at(j));
      }
    }
  }

  // I don't do anything.
  virtual void produce(framework::Event &) {}
};

/**
 * @func removeFile
 * Deletes the file and returns whether the deletion was successful.
 *
 * This is just a helper function during development.
 * Sometimes it is helpful to leave the generated files, so
 * maybe we can make the removal optional?
 */
static bool removeFile(const char *filepath) { return remove(filepath) == 0; }

}  // namespace test
}  // namespace framework

DECLARE_PRODUCER_NS(framework::test, TestConfig)

/**
 * Test for Configure Python class
 *
 * Checks:
 * - pass parameters to Process object
 * - pass parameters to EventProcessors
 * - use arguments to python script on command line
 * - TODO pass histogram info to EventProcessors
 * - TODO pass class objects to EventProcessors
 */
TEST_CASE("Configure Python Test", "[Framework][functionality]") {
  const std::string config_file_name{"config_python_test_config.py"};

  // Arguments to pass to ConfigurePython constructor
  char *args[1];

  // Process handle
  framework::ProcessHandle p;

  // Run a check of the python configuration class without arguments.
  SECTION("No arguments to python script") {
    framework::ConfigurePython cfg(config_file_name, args, 0);
    p = cfg.makeProcess();

    CHECK(p->getPassName() == "test");
  }

  // Update the python config so we can pass the log frequency as a parameter.
  std::ifstream in_file;
  std::ofstream out_file;

  in_file.open(config_file_name.c_str(), std::ios::in | std::ios::binary);

  const std::string config_file_name_arg{
      "/tmp/config_python_test_config_arg.py"};
  out_file.open(config_file_name_arg, std::ios::out | std::ios::binary);
  out_file << in_file.rdbuf();
  out_file << "import sys" << std::endl;
  out_file << "p.logFrequency = int(sys.argv[1])" << std::endl;

  in_file.close();
  out_file.close();

  // Pass the log frequency as a parameter to the process and check that it
  // was set correctly.
  auto correct_log_freq{9000};
  SECTION("Single argument to python script") {
    args[0] = "9000";
    framework::ConfigurePython cfg(config_file_name_arg, args, 1);
    p = cfg.makeProcess();

    CHECK(p->getLogFrequency() == correct_log_freq);
  }

  // add a malformed parameter to test failing
  in_file.open(config_file_name.c_str(), std::ios::in | std::ios::binary);

  out_file.open(config_file_name_arg, std::ios::out | std::ios::binary);
  out_file << in_file.rdbuf();
  out_file << "p.sequence[0].bad_param = ('tuples','are','not','supported')"
           << std::endl;

  in_file.close();
  out_file.close();

  // warning: this test will fail if the repr of a tuple changes format
  SECTION("Bad parameter exception test") {
    REQUIRE_THROWS_WITH(
        std::make_unique<framework::ConfigurePython>(config_file_name_arg, args,
                                                     0),
        ContainsSubstring("('tuples', 'are', 'not', 'supported')"));
    // we need to manually close up our python interpreter
    // because we left during an exception without closing it above
    Py_FinalizeEx();
  }
}
