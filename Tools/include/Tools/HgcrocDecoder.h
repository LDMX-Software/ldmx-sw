#ifndef TOOLS_HGCROCDECODER_H_
#define TOOLS_HGCROCDECODER_H_

#include <map>

// LDMX
#include "Framework/Exception/Exception.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace tools {

/**
 * @class HgcrocDecoder
 * @brief Decodes the buffer output from an HGC ROC into the HgcrocDigiCollection object.
 */
class HgcrocDecoder {
 public:
  /**
   * Class constructor.
   */
  HgcrocDecoder(int version = 2) : roc_version_{version} {}

  /**
   * Decode the input buffer of encoded data into a map
   * of **electron IDs** to the digi samples.
   */
  std::map<uint32_t, std::vector<ldmx::HgcrocDigiCollection::Sample>>
    decode(const std::vector<uint32_t>& encoded_data);

 private:
  /// version of the ROC that is being decoded
  int roc_version_;
};

}  // namespace tools

#endif  // TOOLS_HGCROCDECODER_H_
