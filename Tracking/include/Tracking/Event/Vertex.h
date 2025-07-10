#ifndef TRACKING_EVENT_VERTEX_H_
#define TRACKING_EVENT_VERTEX_H_

//----------------------//
//   C++ Standard Lib   //
//----------------------//
#include <iostream>
#include <optional>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

//----------//
//   LDMX   //
//----------//
#include "Tracking/Event/Track.h"

namespace ldmx {
  
  class Vertex {
  
  public: 
    // Fitted tracks will not be visible in root tree but
    // will be accessible when reading back the rootfile using for example the
    // monitoring code.
    struct FittedTrack {
      double chiSqContrib, distToVtx; 
      std::vector<double> params;      
      std::vector<double> cov;      
      std::vector<double> momentum;
      int pdgId; 
    };
    Vertex(){};
    
    /**
     * Destructor.
     *
     * Currently, the destructor does nothing.
     */
    virtual ~Vertex(){};
    
    /**
     * Print the string representation of this object.
     *
     * This class is needed by ROOT when building the dictionary.
     */
    void Print() const;
    
    // To match the Framework Bus clear. It's doing nothing
    void Clear() {};
    //setters
    void setPosition(std::vector<double> pos){ position_=pos; };
    void setMomentum(std::vector<double> mom){ momentum_=mom; };
    void setCovariance(std::vector<double> cov){ position_cov_=cov; };
    void setTracks(std::vector<Track> tracks){ tracks_=tracks; };
    void setTime(double t){ time_=t; };
    void setMass(double m){ mass_=m; };
    void setChi2(double chi2){ chi2_=chi2; };
    void setNDF(int ndf){ ndf_=ndf; };
    void setPDGID(int id){ pdgID_=id; };
    void addTrack(Track trk){ tracks_.push_back(trk); };
    void addFittedTrack(ldmx::Vertex::FittedTrack ftrk){fittedTracks_.push_back(ftrk); };
  //getters
    std::vector<double> getPosition(){ return position_; };
    std::vector<double> position(){ return position_; };
    std::vector<double> getMomentum(){ return momentum_; };
    std::vector<double> momentum(){ return momentum_; };
    std::vector<double> getCovariance(){ return position_cov_; };
    std::vector<Track> getOriginalTracks(){ return tracks_; };
    std::vector<ldmx::Vertex::FittedTrack> getFittedTracks(){ return fittedTracks_; };
    double getTime(){ return time_; };
    double getMass(){ return mass_; };
    double getChi2(){ return chi2_; };
    int getNDF(){ return ndf_; };
    int getPDGID(){ return pdgID_;};
  protected:
    int n_tracks_{0}; 
    std::vector<double> position_{0., 0., 0.};
    std::vector<double> momentum_{0., 0., 0.};
    std::vector<double> position_cov_;//6-element vectorization of symmetric matrix 
    std::vector<Track> tracks_;
    std::vector<ldmx::Vertex::FittedTrack> fittedTracks_;
   
    double time_{0}; 
    double mass_{0}; 
    double chi2_{0};
    int ndf_{0};
    // pdgID
    int pdgID_{0};    
    
    /// Class declaration needed by the ROOT dictionary.
    ClassDef(Vertex, 1);    
  };
}
#endif  // TRACKING_EVENT_VERTEX_H_
