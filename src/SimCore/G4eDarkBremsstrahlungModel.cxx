/**
 * @file G4eDarkBremsstrahlungModel.cxx
 * @brief Class provided to simulate the dark brem cross section and
 * interaction.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/G4eDarkBremsstrahlungModel.h"
#include "Framework/Exception/Exception.h"
#include "SimCore/G4APrime.h"

// Geant4
#include "G4DataVector.hh"
#include "G4Electron.hh"
#include "G4Element.hh"
#include "G4ElementVector.hh"
#include "G4Material.hh"
#include "G4ParticleChangeForLoss.hh"
#include "G4PhysicalConstants.hh"
#include "G4Positron.hh"
#include "G4ProcessTable.hh"
#include "G4ProductionCutsTable.hh"
#include "G4RunManager.hh"  //for VerboseLevel
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

// Boost
#include <boost/numeric/odeint.hpp>

// STL
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

G4eDarkBremsstrahlungModel::G4eDarkBremsstrahlungModel(
    const G4ParticleDefinition* p, const G4String& theName)
    : G4VEmModel(theName),
      particle_(nullptr),
      fParticleChange_(nullptr),
      method_(DarkBremMethod::Undefined) {
  if (p) {
    SetParticle(p);
  }  // Verify that the particle is an electron.

  MA_ = G4APrime::APrime()->GetPDGMass() / CLHEP::GeV;  // Get the A' mass in
                                                        // GeV
  Mel_ = G4Electron::Electron()->GetPDGMass() / CLHEP::GeV;
}

G4eDarkBremsstrahlungModel::~G4eDarkBremsstrahlungModel() {
  for (G4DataVector* dv : partialSumSigma_) {
    if (dv) delete dv;
  }
  partialSumSigma_.clear();
  madGraphData_.clear();
  currentDataPoints_.clear();
}

void G4eDarkBremsstrahlungModel::Initialise(const G4ParticleDefinition* p,
                                            const G4DataVector& cuts) {
  if (p) {
    SetParticle(p);
  }

  G4double highKinEnergy = this->HighEnergyLimit();
  const G4ProductionCutsTable* theCoupleTable =
      G4ProductionCutsTable::GetProductionCutsTable();

  if (theCoupleTable) {
    G4int numOfCouples = theCoupleTable->GetTableSize();
    G4int nn = partialSumSigma_.size();
    G4int nc = cuts.size();
    if (nn > 0) {
      // reset partialSumSigma
      for (G4DataVector* dv : partialSumSigma_) {
        if (dv) delete dv;
      }
      partialSumSigma_.clear();
    }
    for (G4int i = 0; i < numOfCouples; i++) {
      G4double cute = DBL_MAX;
      if (i < nc) cute = cuts[i];
      const G4MaterialCutsCouple* couple =
          theCoupleTable->GetMaterialCutsCouple(i);
      const G4Material* material = couple->GetMaterial();
      G4DataVector* dv = ComputePartialSumSigma(
          material, 0.5 * highKinEnergy, std::min(cute, 0.25 * highKinEnergy));
      partialSumSigma_.push_back(dv);
    }
  }

  if (not fParticleChange_) fParticleChange_ = GetParticleChangeForLoss();
}

void G4eDarkBremsstrahlungModel::SampleSecondaries(
    std::vector<G4DynamicParticle*>* secondaries, const G4MaterialCutsCouple*,
    const G4DynamicParticle* primary, G4double tmin, G4double maxEnergy) {
  // Deactivate the process after one dark brem. Needs to be reactivated in the
  // end of event action. If this is in the stepping action instead, more than
  // one brem can occur within each step.
  G4bool state = false;
  G4String pname = "biasWrapper(eDBrem)";
  G4ProcessTable* ptable = G4ProcessTable::GetProcessTable();
  ptable->SetProcessActivation(pname, state);

  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 1) {
    std::cout << "[ G4eDarkBremsstrahlungModel ] : A dark brem occurred!"
              << std::endl;
  }

  G4double E0 = primary->GetTotalEnergy();
  G4double tmax = std::min(maxEnergy, E0);
  if (tmin >= tmax) {
    return;
  }                      // limits of the energy sampling
  E0 = E0 / CLHEP::GeV;  // Convert the energy to GeV, the units used in the LHE
                         // files.

  OutgoingKinematics data = GetMadgraphData(E0);
  double EAcc =
      (data.electron.E() - Mel_) / (data.E - Mel_ - MA_) * (E0 - Mel_ - MA_);
  double Pt = data.electron.Pt();
  double P = sqrt(EAcc * EAcc - Mel_ * Mel_);
  double PhiAcc = data.electron.Phi();
  if (method_ == DarkBremMethod::ForwardOnly) {
    unsigned int i = 0;
    while (Pt * Pt + Mel_ * Mel_ > EAcc * EAcc) {
      // Skip events until the Pt is less than the energy.
      i++;
      data = GetMadgraphData(E0);
      EAcc = (data.electron.E() - Mel_) / (data.E - Mel_ - MA_) *
             (E0 - Mel_ - MA_);
      Pt = data.electron.Pt();
      P = sqrt(EAcc * EAcc - Mel_ * Mel_);
      PhiAcc = data.electron.Phi();

      if (i > maxIterations_) {
        std::cout << "[ G4eDarkBremsstrahlungModel ] : "
                  << "Did not manage to simulate with E0 = " << E0
                  << " and EAcc = " << EAcc << std::endl;
        break;
      }
    }
  } else if (method_ == DarkBremMethod::CMScaling) {
    TLorentzVector el(data.electron.X(), data.electron.Y(), data.electron.Z(),
                      data.electron.E());
    double ediff = data.E - E0;
    TLorentzVector newcm(data.centerMomentum.X(), data.centerMomentum.Y(),
                         data.centerMomentum.Z() - ediff,
                         data.centerMomentum.E() - ediff);
    el.Boost(-1. * data.centerMomentum.BoostVector());
    el.Boost(newcm.BoostVector());
    double newE =
        (data.electron.E() - Mel_) / (data.E - Mel_ - MA_) * (E0 - Mel_ - MA_);
    el.SetE(newE);
    EAcc = el.E();
    Pt = el.Pt();
    P = el.P();
  } else if (method_ == DarkBremMethod::Undefined) {
    EAcc = E0;
    P = primary->GetTotalMomentum();
    Pt = sqrt(primary->Get4Momentum().px() * primary->Get4Momentum().px() +
              primary->Get4Momentum().py() * primary->Get4Momentum().py());
  } else {
    EXCEPTION_RAISE("InvalidMethod", "Invallid dark brem simulation method " +
                                         std::to_string(method_) + ".");
  }

  EAcc = EAcc *
         CLHEP::GeV;  // Change the energy back to MeV, the internal GEANT unit.

  G4double momentum =
      sqrt(EAcc * EAcc -
           electron_mass_c2 * electron_mass_c2);  // Electron momentum in MeV.
  G4ThreeVector newDirection;
  double ThetaAcc = std::asin(Pt / P);
  newDirection.set(std::sin(ThetaAcc) * std::cos(PhiAcc),
                   std::sin(ThetaAcc) * std::sin(PhiAcc), std::cos(ThetaAcc));
  newDirection.rotateUz(primary->GetMomentumDirection());
  newDirection.setMag(momentum);

  // create g4dynamicparticle object for the dark photon.
  G4ThreeVector direction = (primary->GetMomentum() - newDirection);
  G4DynamicParticle* dphoton =
      new G4DynamicParticle(G4APrime::APrime(), direction);
  secondaries->push_back(dphoton);

  // energy of primary
  G4double finalKE = EAcc - electron_mass_c2;

  // stop tracking and create new secondary instead of primary
  if (alwaysCreateNewElectron_ or finalKE < SecondaryThreshold()) {
    fParticleChange_->ProposeTrackStatus(fStopAndKill);
    fParticleChange_->SetProposedKineticEnergy(0.0);
    G4DynamicParticle* el =
        new G4DynamicParticle(const_cast<G4ParticleDefinition*>(particle_),
                              newDirection.unit(), finalKE);
    secondaries->push_back(el);
    // continue tracking
  } else {
    // just have primary lose energy (don't rename to different track ID)
    fParticleChange_->SetProposedMomentumDirection(newDirection.unit());
    fParticleChange_->SetProposedKineticEnergy(finalKE);
  }
}

void G4eDarkBremsstrahlungModel::SetMethod(DarkBremMethod method) {
  method_ = method;
  return;
}

void G4eDarkBremsstrahlungModel::SetMadGraphDataFile(std::string file) {
  // TODO do we need to read all LHE files in a directory? Can it just be one
  // LHE file? Read all of the lhe files in the Resources/ directory. Assumes
  // that they are of the correct mass, need to implement method of separating
  // masses (either filenames, or skipping events with incorrect masses).
  //    DIR *dir;
  //    dir = opendir("Resources/");
  //    struct dirent *directory;
  //    if(dir) {
  //        while((directory = readdir(dir)) != NULL) {
  //            std::string fname = "Resources/" +
  //            std::string(directory->d_name);
  //            //Parse files that end in ".lhe"
  //            if(fname.substr(fname.find_last_of(".")+1) == "lhe")
  //            {ParseLHE(fname);}
  //        }
  //    }

  if (file.substr(file.find_last_of(".") + 1) == "lhe") {
    // is an lhe file
    ParseLHE(file);
  } else {
    EXCEPTION_RAISE("LHEFile", "The passed file path '" + file +
                                   "' does not end with 'lhe', so we are "
                                   "assuming it is not an LHE file.");
  }

  MakePlaceholders();  // Setup the placeholder offsets for getting data.

  return;
}

const G4Element* G4eDarkBremsstrahlungModel::SelectRandomAtom(
    const G4MaterialCutsCouple* couple) {
  // select randomly 1 element within the material

  const G4Material* material = couple->GetMaterial();
  G4int nElements = material->GetNumberOfElements();
  const G4ElementVector* theElementVector = material->GetElementVector();

  const G4Element* elm = 0;

  if (1 < nElements) {
    --nElements;
    G4DataVector* dv = partialSumSigma_.at(couple->GetIndex());
    G4double rval = G4UniformRand() * ((*dv)[nElements]);

    elm = (*theElementVector)[nElements];
    for (G4int i = 0; i < nElements; ++i) {
      if (rval <= (*dv)[i]) {
        elm = (*theElementVector)[i];
        break;
      }
    }
  } else {
    elm = (*theElementVector)[0];
  }

  SetCurrentElement(elm);
  return elm;
}

void G4eDarkBremsstrahlungModel::Chi::operator()(const StateType&,
                                                 StateType& dxdt, double t) {
  G4double MUp = 2.79;   // mass up quark [GeV]
  G4double Mpr = 0.938;  // mass proton [GeV]

  G4double d = 0.164 / pow(A, 2. / 3.);
  G4double ap = 773.0 / (Mel * pow(Z, 2. / 3.));
  G4double a = 111.0 / (Mel * pow(Z, 1. / 3.));
  G4double G2el = pow(Z, 2) * pow(a, 4) * pow(t, 2) /
                  (pow(1.0 + a * a * t, 2) * pow(1.0 + t / d, 2));
  G4double G2in = Z * pow(ap, 4) * pow(t, 2) /
                  (pow(1.0 + ap * ap * t, 2) * pow(1.0 + t / 0.71, 8)) *
                  pow(1.0 + t * (pow(MUp, 2) - 1.0) / (4.0 * pow(Mpr, 2)), 2);
  G4double G2 = G2el + G2in;
  G4double ttmin = MA * MA * MA * MA / 4.0 / E0 / E0;
  G4double Under = G2 * (t - ttmin) / t / t;

  dxdt[0] = Under;

  return;
}

void G4eDarkBremsstrahlungModel::DiffCross::operator()(const StateType&,
                                                       StateType& DsigmaDx,
                                                       double x) {
  G4double beta = sqrt(1 - MA * MA / E0 / E0);
  G4double num = 1. - x + x * x / 3.;
  G4double denom = MA * MA * (1. - x) / x + Mel * Mel * x;

  DsigmaDx[0] = beta * num / denom;

  return;
}

void G4eDarkBremsstrahlungModel::SetParticle(const G4ParticleDefinition* p) {
  if (p != G4Electron::Electron()) {
    EXCEPTION_RAISE("InvalidParticle",
                    "The particle passed to G4eDarkBremsstrahlungModel '" +
                        p->GetParticleName() + "' is not an electron.");
  }
  particle_ = p;
}

void G4eDarkBremsstrahlungModel::ParseLHE(std::string fname) {
  // TODO: use already written LHE parser?
  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 0) {
    std::cout << "[ G4eDarkBremsstrahlungModel ] : Parsing LHE file '" << fname
              << "'" << std::endl;
  }
  std::ifstream ifile;
  ifile.open(fname.c_str());
  if (!ifile) {
    EXCEPTION_RAISE("LHEFile", "Unable to open LHE file '" + fname + "'.");
  }

  std::string line;
  while (std::getline(ifile, line)) {
    std::istringstream iss(line);
    int ptype, state;
    double skip, px, py, pz, E, M;
    if (iss >> ptype >> state >> skip >> skip >> skip >> skip >> px >> py >>
        pz >> E >> M) {
      if ((ptype == 11) && (state == -1)) {
        double ebeam = E;
        double e_px, e_py, e_pz, a_px, a_py, a_pz, e_E, a_E, e_M, a_M;
        for (int i = 0; i < 2; i++) {
          std::getline(ifile, line);
        }
        std::istringstream jss(line);
        jss >> ptype >> state >> skip >> skip >> skip >> skip >> e_px >> e_py >>
            e_pz >> e_E >> e_M;
        if ((ptype == 11) && (state == 1)) {  // Find a final state electron.
          for (int i = 0; i < 2; i++) {
            std::getline(ifile, line);
          }
          std::istringstream kss(line);
          kss >> ptype >> state >> skip >> skip >> skip >> skip >> a_px >>
              a_py >> a_pz >> a_E >> a_M;
          if (ptype == 622 and state == 1) {
            if (abs(1. - a_M / MA_) > 1e-3) {
              EXCEPTION_RAISE("BadMGEvnt",
                              "A MadGraph imported event has a different "
                              "APrime mass than the model has (MadGraph = " +
                                  std::to_string(a_M) + "GeV; Model = " +
                                  std::to_string(MA_) + "GeV).");
            }
            OutgoingKinematics evnt;
            double cmpx = a_px + e_px;
            double cmpy = a_py + e_py;
            double cmpz = a_pz + e_pz;
            double cmE = a_E + e_E;
            evnt.electron = TLorentzVector(e_px, e_py, e_pz, e_E);
            evnt.centerMomentum = TLorentzVector(cmpx, cmpy, cmpz, cmE);
            evnt.E = ebeam;
            madGraphData_[ebeam].push_back(evnt);
          }  // get a prime kinematics
        }    // check for final state
      }      // check for particle type and state
    }        // able to get momentum/energy numbers
  }          // while getting lines
  // Add the energy to the list, with a random offset between 0 and the total
  // number of entries.
  ifile.close();
  if (G4RunManager::GetRunManager()->GetVerboseLevel() > 0) {
    printf("[ G4eDarkBremsstrahlungModel ] : Parsed LHE file '%s':\n",
           fname.c_str());
    for (const auto& kV : madGraphData_) {
      printf("                               : %6.4f GeV Beam -> %lu Events\n",
             kV.first, kV.second.size());
    }
  }
}

void G4eDarkBremsstrahlungModel::MakePlaceholders() {
  currentDataPoints_.clear();
  maxIterations_ = 10000;
  for (const auto& iter : madGraphData_) {
    currentDataPoints_[iter.first] = int(G4UniformRand() * iter.second.size());
    if (iter.second.size() < maxIterations_)
      maxIterations_ = iter.second.size();
  }
}

G4eDarkBremsstrahlungModel::OutgoingKinematics
G4eDarkBremsstrahlungModel::GetMadgraphData(double E0) {
  OutgoingKinematics cmdata;  // data frame to return

  // Cycle through imported beam energies until the closest one above is found,
  // or the max is reached.
  double samplingE = 0.;
  for (const auto& keyVal : currentDataPoints_) {
    samplingE = keyVal.first;  // move samplingE up
    // check if went under the sampling energy
    //  the map is sorted by key, so we can be done right after E0 goes under
    //  samplingE
    if (E0 < samplingE) break;
  }
  // now samplingE is the closest energy above E0 or the maximum energy imported
  // from mad graph

  // Need to loop around if we hit the end, when the size of
  // madGraphData_[samplingE] is smaller than
  //  the number of events we want
  if (currentDataPoints_.at(samplingE) >= madGraphData_.at(samplingE).size()) {
    currentDataPoints_[samplingE] = 0;
  }

  // Get the lorentz vectors from the index given by the placeholder.
  cmdata = madGraphData_.at(samplingE).at(currentDataPoints_.at(samplingE));

  // Increment the current index
  currentDataPoints_[samplingE]++;

  return cmdata;
}

G4double G4eDarkBremsstrahlungModel::ComputeCrossSectionPerAtom(
    const G4ParticleDefinition*, G4double E0, G4double Z, G4double A,
    G4double cut, G4double) {
  if (E0 < keV or E0 < cut) return 0.;  // outside viable region for model

  E0 = E0 / CLHEP::GeV;  // Change energy to GeV.

  if (E0 < 2. * MA_) return 0.;  // can't produce a prime

  // begin: chi-formfactor calculation
  Chi chiformfactor;
  //  set parameters
  chiformfactor.A = A;
  chiformfactor.Z = Z;
  chiformfactor.E0 = E0;
  chiformfactor.MA = MA_;
  chiformfactor.Mel = Mel_;

  double tmin = MA_ * MA_ * MA_ * MA_ / (4. * E0 * E0);
  double tmax = MA_ * MA_;

  // Integrate over chi.
  StateType integral(1);
  integral[0] = 0.;  // start integral value at zero
  boost::numeric::odeint::integrate(
      chiformfactor  // how to calculate integrand
      ,
      integral  // integral result
      ,
      tmin  // integral lower limit
      ,
      tmax  // integral upper limit
      ,
      (tmax - tmin) / 1000  // dt - initial, adapts based off error
  );

  G4double ChiRes = integral[0];

  // Integrate over x. Can use log approximation instead, which falls off at
  // high A' mass.
  DiffCross diffcross;
  diffcross.E0 = E0;
  diffcross.MA = MA_;
  diffcross.Mel = Mel_;

  double xmin = 0;
  double xmax = 1;
  if ((Mel_ / E0) > (MA_ / E0))
    xmax = 1 - Mel_ / E0;
  else
    xmax = 1 - MA_ / E0;

  // Integrate over differential cross section.
  integral[0] = 0.;  // start integral value at zero
  boost::numeric::odeint::integrate(
      diffcross  // how to calculate integrand
      ,
      integral  // integral result
      ,
      xmin  // integral lower limit
      ,
      xmax  // integral upper limit
      ,
      (xmax - xmin) / 1000  // dx - initial, adapts based off error
  );

  G4double DsDx = integral[0];

  G4double GeVtoPb = 3.894E08;
  G4double alphaEW = 1.0 / 137.0;
  G4double epsilBench = 1;

  G4double cross = GeVtoPb * 4. * alphaEW * alphaEW * alphaEW * epsilBench *
                   epsilBench * ChiRes * DsDx * CLHEP::picobarn;

  if (cross < 0.) return 0.;  // safety check all the math

  return cross;
}

G4DataVector* G4eDarkBremsstrahlungModel::ComputePartialSumSigma(
    const G4Material* material, G4double kineticEnergy, G4double cut) {
  G4int nElements = material->GetNumberOfElements();
  const G4ElementVector* theElementVector = material->GetElementVector();
  const G4double* theAtomNumDensityVector =
      material->GetAtomicNumDensityVector();
  G4DataVector* dv = new G4DataVector();  // stored into partialSumSigma_ and
                                          // then cleaned up later
  G4double cross = 0.0;

  for (G4int i = 0; i < nElements; i++) {
    cross += theAtomNumDensityVector[i] *
             ComputeCrossSectionPerAtom(
                 particle_, kineticEnergy, (*theElementVector)[i]->GetZ(),
                 (*theElementVector)[i]->GetA(), cut, 0.);
    dv->push_back(cross);
  }

  return dv;
}
