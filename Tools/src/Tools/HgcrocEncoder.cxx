
#include "Tools/HgcrocEncoder.h"

#include <bitset>

#include "Packing/Utility/CRC.h"
#include "Packing/Utility/Mask.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Tools/BufferReader.h"

namespace tools {

std::vector<uint32_t> HgcrocEncoder::encode(
    const std::map<uint32_t, std::vector<ldmx::HgcrocDigiCollection::Sample>>&
        decoded_data) {
  /**
   * Static parameters depending on ROC version
   */
  static const unsigned int common_mode_channel = roc_version_ == 2 ? 19 : 1;

  std::vector<uint32_t> buffer;

  return buffer;
}

}  // namespace tools

