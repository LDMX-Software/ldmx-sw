#ifndef DISCRETEINPUTS_H
#define DISCRETEINPUTS_H

#include <stdint.h>
#include <stdio.h>

#include <vector>

namespace trigger {

namespace ldmx_int {

struct EcalTP {
  uint8_t tp;
  uint32_t tid;
  // extra data for added convenience
  uint32_t tp_lin;
  uint32_t layer;
  uint32_t module;
  uint32_t cell;
  
  bool operator<(const EcalTP &other) const { return tp > other.tp; }
  void fill(int _tid, int _tp) {
    tid = _tid;
    tp = _tp;
    // derived data, optional
    layer   = 0;
    module  = 0;
    cell    = 0;
    tp_lin  = 0;
  }
  void fill(int _tid, int _tp, int _layer, int _module, int _cell, int _tp_lin) {
    tid     = _tid;
    tp      = _tp;
    layer   = _layer;
    module  = _module;
    cell    = _cell;
    tp_lin  = _tp_lin;
  }
  void writeToFile(FILE *file) const {
    fwrite(&tp, sizeof(uint8_t), 1, file);
    fwrite(&tid, sizeof(uint32_t), 1, file);
    fwrite(&layer , sizeof(uint32_t), 1, file);
    fwrite(&module, sizeof(uint32_t), 1, file);
    fwrite(&cell  , sizeof(uint32_t), 1, file);
    fwrite(&tp_lin, sizeof(uint32_t), 1, file);
  }
  void readFromFile(FILE *file) {
    fread(&tp, sizeof(uint8_t), 1, file);
    fread(&tid, sizeof(uint32_t), 1, file);
    fread(&layer , sizeof(uint32_t), 1, file);
    fread(&module, sizeof(uint32_t), 1, file);
    fread(&cell  , sizeof(uint32_t), 1, file);
    fread(&tp_lin, sizeof(uint32_t), 1, file);
  }
};

template <typename T>
void writeManyToFile(const std::vector<T> &objs, FILE *file) {
  uint32_t number = objs.size();
  fwrite(&number, 4, 1, file);
  for (uint32_t i = 0; i < number; ++i) objs[i].writeToFile(file);
}

template <typename T>
void readManyFromFile(std::vector<T> &objs, FILE *file) {
  uint32_t number;
  fread(&number, 4, 1, file);
  objs.resize(number);
  for (uint32_t i = 0; i < number; ++i) objs[i].readFromFile(file);
}

}  // namespace ldmx_int

}  // namespace trigger

#endif /* DISCRETEINPUTS_H */
