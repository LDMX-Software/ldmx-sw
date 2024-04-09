#include "Framework/EventProcessor.h"
#include "SimCore/Event/SimParticle.h"

namespace dqm {
/**
 * @class DarkBremInteraction
 *
 * Go through the particle map and find the dark brem products,
 * storing their vertex and the dark brem outgoing kinematics
 * for further study.
 *
 * While histograms are filled to be automatically validated and plotted,
 * we also put these values into the event tree so users can look at the
 * variables related to the dark brem in detail.
 *
 * ## Products
 * APrime{Px,Py,Pz} - 3-vector momentum of A' at dark brem
 * APrimeEnergy   - energy of A' at dark brem
 * Recoil{Px,Py,Pz} - 3-vector momentum of electron recoiling from dark brem
 * RecoilEnergy   - energy of recoil at dark brem
 * Incident{Px,Py,Pz} - 3-vector momentum of electron incident to dark brem
 * IncidentEnergy   - energy of incident electron at dark brem
 * APrimeParentID - TrackID of A' parent
 * DarkBremVertexMaterial - integer corresponding to index of known_materials 
 *                          parameter OR -1 if not found in known_materials
 * DarkBremVertexMaterialZ - elemental Z value for element chosen by random from
 *                           the elements in the material
 * DarkBrem{X,Y,Z} - physical space location where dark brem occurred
 */
class DarkBremInteraction : public framework::Producer {
 public:
  DarkBremInteraction(const std::string& n, framework::Process& p)
    : framework::Producer(n,p) {}
  /**
   * update the labels of some categorial histograms
   *
   * This is helpful for downstream viewers of the histograms
   * so that ROOT will display the bins properly.
   */
  virtual void onProcessStart() final override;

  /**
   * extract the kinematics of the dark brem interaction from the SimParticles
   *
   * Sometimes the electron that undergoes the dark brem is not in a region 
   * where it should be saved (i.e. it is a shower electron inside of the ECal).
   * In this case, we need to reconstruct the incident momentum from the outgoing
   * products (the recoil electron and the dark photon) which should be saved by
   * the biasing filter used during the simulation.
   *
   * Since the dark brem model does not include a nucleus, it only is able to 
   * conserve momentum, so we need to reconstruct the incident particle's 3-momentum
   * and then use the electron mass to calculate its total energy.
   */
  virtual void produce(framework::Event& e) final override;
 private:
  /**
   * Set the labels of the histogram of the input name with the input labels
   */
  void setHistLabels(const std::string& name, const std::vector<std::string>& labels);

  /**
   * the list of known materials assigning them to material ID numbers
   *
   * During the simulation, we can store the name of the logical volume
   * that the particle originated in. There can be many copies of logical
   * volumes in different places but they all will be the same material
   * by construction of how we designed our GDML. In the ecal GDML, the
   * beginning the 'volume' tags list the logical volumes and you can
   * see there which materials they all are in.
   *
   * We go through this list on each event, checking if any of these entries
   * match a substring of the logical volume name stored. If we don't find any,
   * the integer ID is set to -1.
   *
   * The inverse LUT that can be used on the plotting side is
   * 
   *    material_lut = {
   *      0 : 'Unknown',
   *      1 : 'C',
   *      2 : 'PCB',
   *      3 : 'Glue',
   *      4 : 'Si',
   *      5 : 'Al',
   *      6 : 'W',
   *      7 : 'PVT'
   *    }
   *
   * This is kind of lazy, we could instead do a full LUT where we list all known
   * logical volume names and their associated materials but this analysis isn't
   * as important so I haven't invested that much time in it yet.
   */
  std::map<std::string, int> known_materials_ = {
    { "Carbon", 1 },
    { "PCB", 2 }, // in v12, the motherboards were simple rectangles with 'PCB' in the name
    { "Glue", 3 },
    { "Si", 4 },
    { "Al", 5 },
    { "W" , 6 },
    { "target", 6 },
    { "trigger_pad", 7 },
    { "strongback" , 5 }, // strongback is made of aluminum
    { "motherboard" , 2 }, // motherboards are PCB
    { "support" , 5 }, // support box is aluminum
    { "CFMix" , 3 }, // in v12, we called the Glue layers CFMix
    { "C_volume" , 1 } // in v12, we called the carbon cooling planes C but this is too general for substr matching
  };

  /**
   * The list of known elements assigning them to the bins that we are putting them into.
   *
   * There are two failure modes for this:
   * 1. The dark brem didn't happen, in which case, the element reported by the event header
   *    will be -1. We give this an ID of 0.
   * 2. The dark brem occurred within an element not listed here, in which case we give it
   *    the last bin.
   *
   * The inverset LUT that can be used if studying the output tree is
   *
   *    element_lut = {
   *      0 : 'did_not_happen',
   *      1 : 'H 1',
   *      2 : 'C 6',
   *      3 : 'O 8',
   *      4 : 'Na 11',
   *      5 : 'Si 14',
   *      6 : 'Ca 20',
   *      7 : 'Cu 29',
   *      8 : 'W 74',
   *      9 : 'unlisted'
   *    }
   */
  std::map<int, int> known_elements_ = {
    {1, 1},
    {6, 2},
    {8, 3},
    {11, 4},
    {14, 5},
    {20, 6},
    {29, 7},
    {74, 8}
  };
};

}
