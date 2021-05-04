
#include "SimCore/Geo/ParserFactory.h"

//---< SimCore >---//
#include "SimCore/Geo/Parser.h"
#include "SimCore/Geo/GDMLParser.h"

namespace simcore {
namespace geo {

ParserFactory *ParserFactory::instance_ = nullptr;

ParserFactory *ParserFactory::getInstance() {
  if (!instance_)
    instance_ = new ParserFactory;
  return instance_;
}

ParserFactory::ParserFactory() { registerParser("gdml", &GDMLParser::create); }

void ParserFactory::registerParser(const std::string &name, createFunc create) {
  parser_map[name] = create;
}

Parser *ParserFactory::createParser(const std::string &name,
                                    framework::config::Parameters &parameters, 
				    simcore::ConditionsInterface &ci) {
  auto it{parser_map.find(name)};
  if (it != parser_map.end())
    return it->second(parameters, ci);
  return nullptr;
}

} // namespace geo
} // namespace simcore
