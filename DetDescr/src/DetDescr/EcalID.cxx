#include "DetDescr/EcalID.h"

#include "DetDescr/DetectorIDInterpreter.h"

namespace ldmx {

std::ostream& operator<<(std::ostream& s, const ldmx::EcalID& id) {
  std::pair uv = id.getCellUV();
  s << "Ecal(" << id.layer() << ',' << id.module() << ',' << id.cell() << '['
    << uv.first << ',' << uv.second << "])";
  return s;
}

void EcalID::createInterpreters() {
  IDField::IDFieldList fields;
  fields.push_back(new IDField("subdetector", 0, SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("layer", 1, LAYER_SHIFT,
                  LAYER_SHIFT + IDField::countOnes(LAYER_MASK) - 1));
  fields.push_back(
      new IDField("module", 2, MODULE_SHIFT,
                  MODULE_SHIFT + IDField::countOnes(MODULE_MASK) - 1));
  fields.push_back(new IDField("cell", 3, CELL_SHIFT,
                               CELL_SHIFT + IDField::countOnes(CELL_MASK) - 1));

  DetectorIDInterpreter::registerInterpreter(
      SD_ECAL,
      EcalAbstractID::CELL_TYPE_MASK << EcalAbstractID::CELL_TYPE_SHIFT,
      EcalAbstractID::PrecisionGlobal << EcalAbstractID::CELL_TYPE_SHIFT,
      fields);
  DetectorIDInterpreter::registerInterpreter(
      SD_ECAL,
      EcalAbstractID::CELL_TYPE_MASK << EcalAbstractID::CELL_TYPE_SHIFT,
      EcalAbstractID::PrecisionLocal << EcalAbstractID::CELL_TYPE_SHIFT,
      fields);
}

static const unsigned int BASE_ROW_W = 13;
static const unsigned int V_MIDDLE = 11;
static const unsigned int MAX_V = 23;

EcalID::EcalID(unsigned int layer, unsigned int module, unsigned int u,
               unsigned int v)
    : EcalAbstractID(EcalAbstractID::PrecisionGlobal, 0) {
  id_ |= (layer & LAYER_MASK) << LAYER_SHIFT;
  id_ |= (module & MODULE_MASK) << MODULE_SHIFT;

  unsigned int cell = 0;
  if (v > MAX_V) {
    EXCEPTION_RAISE("InvalidIdException",
                    "Attempted to create EcalID with invalid (u,v)=(" +
                        std::to_string(u) + "," + std::to_string(v) + ")");
  }

  if (v <= V_MIDDLE) {  // simple case...
    if (u > (BASE_ROW_W - 1 + v)) {
      EXCEPTION_RAISE("InvalidIdException",
                      "Attempted to create EcalID with invalid (u,v)=(" +
                          std::to_string(u) + "," + std::to_string(v) + ")");
    }
    cell = u + v * BASE_ROW_W + (v - 1) * v / 2;
  } else {
    unsigned int umin = v - V_MIDDLE;
    static unsigned int umax = 23;  // constant
    unsigned int vrel = v - V_MIDDLE - 1;
    if (u < umin || u > umax) {
      EXCEPTION_RAISE("InvalidIdException",
                      "Attempted to create EcalID with invalid (u,v)=(" +
                          std::to_string(u) + "," + std::to_string(v) + ")");
    }
    cell = 222 + (u - umin) + vrel * BASE_ROW_W + 55 -
           (V_MIDDLE - vrel - 1) * (V_MIDDLE - vrel) / 2;
  }
  id_ |= (cell & CELL_MASK) << CELL_SHIFT;
}

static const int ROW_STARTS[25] = {0,   13,  27,  42,  58,  75,  93,  112, 132,
                                   153, 175, 198, 222, 245, 267, 288, 308, 327,
                                   345, 362, 378, 393, 407, 420, -1};

std::pair<unsigned int, unsigned int> EcalID::getCellUV() const {
  int cell = getCellID();
  unsigned int v;
  for (v = 0; v < MAX_V && cell >= ROW_STARTS[v + 1];
       v++);  // find the right v value
  unsigned int u = cell - ROW_STARTS[v];
  if (v > V_MIDDLE) u += (v - V_MIDDLE);
  return std::pair<unsigned int, unsigned int>(u, v);
}
}  // namespace ldmx
