
#include "SimCore/Geo/ParserFactory.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/Geo/GDMLParser.h"

namespace simcore {
namespace geo {

ParserFactory *ParserFactory::getInstance() {
  if (!instance)
    instance = new ParserFactory;
  return instance;
}

ParserFactory::ParserFactory() { registerParser("gdml", &GDMLParser::create); }

void ParserFactory::registerParser(const std::string &name, createFunc create) {
  parser_map[name] = create;
}

Parser *ParserFactory::createParser(const std::string &name,
                                    ldmx::Parameters &parameters) {
  auto it{parser_map.find(name)};
  if (it != parser_map.end())
    return it->second(parameters);
  return nullptr;
}

} // namespace geo
} // namespace simcore
