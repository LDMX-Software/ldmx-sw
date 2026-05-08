#pragma once

#include <array>
#include <bitset>
#include <span>

#include "Framework/Logger.h"

namespace packing {

/**
 * Single ECON-D event packet
 *
 * The ECON-D is the CONcentrator mezzanine for the Data path
 * and it has many different configuration modes.
 * The two most important to us for decoding are
 * 1. "Pass Through" mode - all 37 sample words are copied from the link,
 *    the link headers from the ROC are overwritten by ECOND link headers
 * 2. "Regular" mode - the ECOND event format is written which can include
 *    zero-suppression of multiple varieties including full eliminating
 *    channels that are empty
 *
 * The ECONDFormatter emulates the ECON-D re-formatting of the HGCROC
 * event packets into the ECON-D event packet. This unpacking is done
 * separately from this formatter so that it can be applied to data
 * actually extracted from an ECON-D.
 */
class ECONDEventPacket {
  /// handle to logging source
  enableLogging("ECONDEventPacket");

  /**
   * Unpack a eRx ("link") subpacket from the ECOND event into a DAQLinkFrame
   *
   * @note We overload the zero'th corruption bit for DAQLinkFrame to mean
   * that any of the three Stat bits are bad (not 1).
   *
   * @param[in] data subpacket data to unpack
   * @param[out] link DAQLinkFrame to update with unpacked data
   * @paran[in] passthrough whether the ECOND is in pass through mode or not
   * @return std::size_t length of data in 32b words that was unpacked
   */
  std::size_t unpack_link_subpacket(std::span<uint32_t> data,
                                    DAQLinkFrame& link, bool passthrough);

 public:
  /**
   * storage of corruption bits
   *
   * Index | Description
   * ------|------------
   *  0    | 9b header marker is not the ECOND spec or common CMS config
   *  1    | stored length does not equal packet length
   *  2    | 8b header CRC mismatch
   *  3    | 32b sub-packet CRC mismatch
   */
  std::array<bool, 4> corruption;

  /**
   * storage of unpacked link data
   *
   * This is very space in-efficient since we store zero words for
   * any channel or measurement that was zero suppressed.
   * This won't be a problem for the smaller-scale testbeam prototypes,
   * but could be a memory issue when we get to the full LDMX
   * detector with its almost 100k channels.
   */
  std::vector<DAQLinkFrame> links;

  /**
   * Construct an event packet with the number of eRx "links"
   */
  ECONDEventPacket(std::size_t n_links);

  /// get ADC readout from common mode 0 of input link
  int adc_cm0(int i_link) const;

  /// get ADC readout from common mode 1 of input link
  int adc_cm1(int i_link) const;

  /// get channel Sample from a link
  Sample channel(int i_link, int i_ch) const;

  /// get calib Sample from a link half
  Sample calib(int i_link) const;

  /**
   * parse into the packet from the passed data span
   *
   * This function is what implements the format describe
   * in the ECOND specification.
   *
   * @see MultiSampleECONDEventPacket for wrapping this function with more
   * unpacking to handle the extra headers and trailers that are added
   * by software emulation or hardware.
   */
  void from(std::span<uint32_t> data);
};

}  // namespace pflib::packing
