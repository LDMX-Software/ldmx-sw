#ifndef SIMCORE_GEO_PARSERFACTORY_H_
#define SIMCORE_GEO_PARSERFACTORY_H_

#include "SimCore/Geo/Parser.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

namespace simcore {
namespace geo {

class ParserFactory {
 public:
  /// Get the instance to this factory
  static ParserFactory &getInstance();

  /// Default constructor
  ~ParserFactory() = default;

  /**
   * Create an instance of the parser of the given type.
   *
   * @param[in] type String type of the perser that needs to be created.
   * @param[in] ci Interface to conditions system.
   */
  Parser *createParser(const std::string &name,
                       framework::config::Parameters &parameters,
                       simcore::ConditionsInterface &ci);

  // Delete the following methods to make sure they are inaccesible.
  ParserFactory(ParserFactory const &) = delete;
  void operator=(ParserFactory const &) = delete;

 private:
  /// Default constructor
  ParserFactory();

  /**
   * Register the parser with this factory.
   *
   * This is used to map the name of the parser to the function used to create
   * it.
   *
   * @param[in] name Name of the parser being registered.
   * @param[in] crate Function used to create this function.
   */
  void registerParser(const std::string &name, createFunc create);

  // Mapping between a parser type and its create function
  std::map<std::string, createFunc> parser_map;
};
}  // namespace geo
}  // namespace simcore

#endif  // SIMCORE_GEO_PARSERFACTORY_H_
