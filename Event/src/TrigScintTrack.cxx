/**                                                                                                                                                                               
 * @file TrigScintTrack.cxx 
 * @brief Class that stores reconstructed track information from the Trigger scintillator pads
 * @author Lene Kristian Bryngemark, Stanford University
 */

#include "Event/TrigScintTrack.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

ClassImp(ldmx::TrigScintTrack)

namespace ldmx {

  void TrigScintTrack::Clear() { //Option_t *option) {
    centroid_=0;
    residual_=0;
  }

  void TrigScintTrack::Print() const { //Option_t *option) const {
    std::cout << "TrigScintTrack { " << " channel centroid: " << getCentroid() 
	      << ",  residual: " << getResidual() << " }" << std::endl;
  }

}
