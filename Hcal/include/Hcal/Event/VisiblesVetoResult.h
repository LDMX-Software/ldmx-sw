/*
 *@file VisiblesVetoResult.h
 *@brief Class used to encapsulate the results obstained
         from VisiblesVetoProcessor
 *@author Tyler Horoho, University of Virginia
 */

#ifndef EVENT_VISIBLESVETORESULT_H_
#define EVENT_VISIBLESVETORESULT_H_

#include <iostream>

//   ROOT   //
#include <TObject.h>

namespace ldmx {

  class VisiblesVetoResult {
  public:
    /** Constructor */
    VisiblesVetoResult();

    /** Destructor */
    virtual ~VisiblesVetoResult();

    void setVariables(int nLayersHit,
		      double xStd, double yStd, double zStd,
		      double xMean, double yMean, double rMean,
		      int isoHits, double isoEnergy,
		      int nReadoutHits, double summedDet,
		      double rMeanFromPhotonProj);

    void Clear();

    void Print() const;

    bool passesVeto() const { return passesVeto_; }

    double getDisc() const { return discValue_; }

    int getNLayersHit() const { return nLayersHit_; }

    double getXStd() const { return xStd_; }

    double getYStd() const { return yStd_; }

    double getZStd() const { return zStd_; }

    double getXMean() const { return xMean_; }

    double getYMean() const { return yMean_; }

    double getRMean() const { return rMean_; }

    int getIsoHits() const { return isoHits_; }

    double getIsoEnergy() const { return isoEnergy_; }

    int getNReadoutHits() const { return nReadoutHits_; }

    double getSummedDet() const { return summedDet_; }

    double getDistFromPhotonProj() const { return rMeanFromPhotonProj_; }

  private:
    bool passesVeto_{false};

    int nLayersHit_{0};
    double xStd_{0};
    double yStd_{0};
    double zStd_{0};
    double xMean_{0};
    double yMean_{0};
    double rMean_{0};
    int isoHits_{0};
    double isoEnergy_{0};
    int nReadoutHits_{0};
    double summedDet_{0};
    double rMeanFromPhotonProj_{0};

    double discValue_{0};


    ClassDef(VisiblesVetoResult, 7);
  };
} // namespace ldmx

#endif
