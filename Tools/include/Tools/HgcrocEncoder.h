#ifndef TOOLS_HGCROCENCODER_H_
#define TOOLS_HGCROCENCODER_H_

#include <map>

// LDMX
#include "Recon/Event/HgcrocDigiCollection.h"

namespace tools {

/**
 * @class HgcrocEncoder
 * @brief Encodes the buffer output from an HGC ROC into the
 * HgcrocDigiCollection object.
 */
class HgcrocEncoder {
 public:
  /**
   * Class constructor.
   */
  HgcrocEncoder(int version = 2) : roc_version_{version} {}

  /**
   * Encode the input data into a buffer.
   *
   * The input data should be provided as a map of **electronic** ID numbers
   * to the samples that were taken.
   */
  std::vector<uint32_t> encode(
      const std::map<uint32_t, std::vector<ldmx::HgcrocDigiCollection::Sample>>&
          decoded_data);

 private:
  /// version of the ROC that is being decoded
  int roc_version_;
};

}  // namespace tools

#endif  // TOOLS_HGCROCENCODER_H_
