#ifndef RECON_HGCROCUNPACKER_H_
#define RECON_HGCROCUNPACKER_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace recon {

/**
 * @class HgcrocUnpacker
 * @brief Decodes the buffer output from an HGC ROC into the HgcrocDigiCollection object.
 */
class HgcrocUnpacker : public framework::Producer {
 public:
  /**
   * Class constructor.
   */
  HgcrocUnpacker(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  /**
   * Class destructor.
   */
  virtual ~HgcrocUnpacker() = default;

  /**
   * Configure the unpacker. We need to know the input buffer name and 
   * the output HgcrocDigiCollection name.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  /**
   */
  virtual void produce(framework::Event& event);

 private:
  /// input buffer name to retrieve from event bus
  std::string input_name_;
  /// input pass name to retrieve from event bus
  std::string input_pass_;
  /// output name of digi collection object
  std::string output_name_;
  /// version of the ROC that is being decoded
  int roc_version_;
};

}  // namespace recon

#endif
