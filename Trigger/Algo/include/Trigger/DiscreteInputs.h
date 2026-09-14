#ifndef DISCRETEINPUTS_H
#define DISCRETEINPUTS_H

#include <stdint.h>
#include <stdio.h>

#include <vector>

namespace trigger {

namespace ldmx_int {

struct EcalTP {
  uint8_t tp_;
  uint32_t tid_;
  // extra data for added convenience
  uint32_t tp_lin_;
  uint32_t layer_;
  uint32_t module_;
  uint32_t cell_;

  bool operator<(const EcalTP& other) const { return tp_ > other.tp_; }
  void fill(int _tid, int _tp) {
    tid_ = _tid;
    tp_ = _tp;
    // derived data, optional
    layer_ = 0;
    module_ = 0;
    cell_ = 0;
    tp_lin_ = 0;
  }
  void fill(int _tid, int _tp, int _layer, int _module, int _cell,
            int _tp_lin) {
    tid_ = _tid;
    tp_ = _tp;
    layer_ = _layer;
    module_ = _module;
    cell_ = _cell;
    tp_lin_ = _tp_lin;
  }
  void writeToFile(FILE* file) const {
    fwrite(&tp_, sizeof(uint8_t), 1, file);
    fwrite(&tid_, sizeof(uint32_t), 1, file);
    fwrite(&layer_, sizeof(uint32_t), 1, file);
    fwrite(&module_, sizeof(uint32_t), 1, file);
    fwrite(&cell_, sizeof(uint32_t), 1, file);
    fwrite(&tp_lin_, sizeof(uint32_t), 1, file);
  }
  void readFromFile(FILE* file) {
    fread(&tp_, sizeof(uint8_t), 1, file);
    fread(&tid_, sizeof(uint32_t), 1, file);
    fread(&layer_, sizeof(uint32_t), 1, file);
    fread(&module_, sizeof(uint32_t), 1, file);
    fread(&cell_, sizeof(uint32_t), 1, file);
    fread(&tp_lin_, sizeof(uint32_t), 1, file);
  }
};

template <typename T>
void writeManyToFile(const std::vector<T>& objs, FILE* file) {
  uint32_t number = objs.size();
  fwrite(&number, 4, 1, file);
  for (uint32_t i = 0; i < number; ++i) objs[i].writeToFile(file);
}

template <typename T>
void readManyFromFile(std::vector<T>& objs, FILE* file) {
  uint32_t number;
  fread(&number, 4, 1, file);
  objs.resize(number);
  for (uint32_t i = 0; i < number; ++i) objs[i].readFromFile(file);
}

}  // namespace ldmx_int

}  // namespace trigger

#endif /* DISCRETEINPUTS_H */
