#ifndef DISCRETEINPUTS_H
#define DISCRETEINPUTS_H
  
namespace trigger {

namespace ldmx_int {

  struct EcalTP {
    uint32_t tid;
    uint32_t tp; // store linear version
    //uint8_t tp;
	
    bool operator<(const EcalTP &other) const { return tp > other.tp; }
    void fill(int _tid, float _tp){
      tid	= _tid;
      tp	= _tp;
    }
    void writeToFile(FILE *file) const {
      fwrite(&tid, sizeof(uint32_t), 1, file);
      fwrite(&tp,  sizeof(uint32_t), 1, file);
    }
    void readFromFile(FILE *file) {
      fread(&tid, sizeof(uint32_t), 1, file);
      fread(&tp,  sizeof(uint32_t), 1, file);
    }
  };
    
  template<typename T>
      void writeManyToFile(const std::vector<T> & objs, FILE *file) {
    uint32_t number = objs.size(); 
    fwrite(&number, 4, 1, file);
    for (uint32_t i = 0; i < number; ++i) objs[i].writeToFile(file);
  }

  template<typename T>
      void readManyFromFile(std::vector<T> & objs, FILE *file) {
    uint32_t number;
    fread(&number, 4, 1, file);
    objs.resize(number); 
    for (uint32_t i = 0; i < number; ++i) objs[i].readFromFile(file);
  }

}

}

#endif /* DISCRETEINPUTS_H */
