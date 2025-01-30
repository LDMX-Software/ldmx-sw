"""Macro for training visibles search BDT

   author Tyler Horoho & Kieran Wall, University of Virginia

   Examples
   --------
   come up with an example
"""

from LDMX.Framework import EventTree
from LDMX.Framework import ldmxcfg
#from LDMX.DetDescr import EcalGeometry

## python packages ##
import matplotlib as plt.
import xgboost as xgb
import numpy as np
from sklearn import metrics
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score

############################################################

class sampleContainer :
    def __init__(self, dn, maxEvts, trainFrac, isBkg):
        print("initializing Container!")

        self.maxEvts = maxEvts
        self.trainFrac = trainFrac
        self.isBkg = isBkg
        self.events = []
        evtcount = 0
        for fn in os.listdir(dn) :
            fp = os.path.join(dn, fn)
            tree = EventTree.EventTree(fp)

            for event in tree:
                evt = []
                if isBkg :
                    EcalRecHits = event.EcalRecHits_sim
                    HcalRecHits = event.HcalRecHits_sim
                    SimParticles = event.SimParticles_sim
                    TargetScoringPlaneHits = event.TargetScoringPlaneHits_sim

                else :
                    EcalRecHits = event.EcalRecHits_v14
                    HcalRecHits = event.HcalRecHits_v14
                    SimParticles = event.SimParticles_v14
                    TargetScoringPlaneHits = event.TargetScoringPlaneHits_v14

                # Check if trigger requirement and othe cuts are met
                Eupstream = 0.
                Eecal = 0.
                Ehcal = 0.
                for hit in EcalRecHits :
                    Eecal += hit.getEnergy()
                    if hit.getZPos() < 541.722 : # hard-coded v14
                        Eupstream += hit.getEnergy()
                if not Eupstream < 3160:
                    continue
                if not Eecal < 3160:
                    continue

                for hit in HcalRecHits :
                    if hit.getZPos() > 870:
                        Ehcal += 12.2*hit.getEnergy()
                if not Ehcal > 4840:
                    continue

                recoilE = 8000.
                eList = []
                for sphit in TargetScoringPlaneHits:
                    if sphit.getPosition()[2] > 0:
                        for it in SimParticles:
                            if it.first == sphit.getTrackID():
                                if it.second.getPdgID() == 11:
                                    eList.append(sphit.getEnergy())
                if len(eList) > 0:
                    recoilE = max(eList)

                if not recoilE < 2400:
                    continue

                ## all cuts passed, now build BDT features ##

                layersHit = []

                hits = 0
                isoHits = 0
                isoE = 0

                xMean = 0
                yMean = 0
                zMean = 0
                rMean = 0

                xStd = 0
                yStd = 0
                zStd = 0

                for it in SimParticles :
                    for sphit in TargetScoringPlaneHits :
                        if sphit.getPosition()[2] > 0 :
                            if it.first == sphit.getTrackID() :
                                if isBkg :
                                    if sphit.getPdgID() == 11 and 0 in it.second.getParents():
                                        x0_gamma = sphit.getPosition()
                                        p_gamma = [-sphit.getMomentum()[0],
                                                   -sphit.getMomentum()[1],
                                                   8000 - sphit.getMomentum()[2]]
                                    else :
                                        if sphit.getPdgID() == 622 :
                                            x0_gamma = sphit.getPosition()
                                            p_gamma = sphit.getMomentum
                rMean_gammaProj = 0.

                for hit in HcalRecHits :
                    if hit.getZPos() > 870 :
                        hits += 1
                        x = hit.getXPos()
                        y = hit.getYPos()
                        z = hit.getZPos()
                        r = math.sqrt(x*x + y*y)

                        energy = 12.2*hit.getEnergy()
                        
                        xmean += x*energy
                        ymean += y*energy
                        zmean += z*energy

                        if not z in layersHit :
                            layersHit.append(z)

                        x_proj = x0_gamma[0] + (z - x0_gamma[2])*p_gamma[0]/p_gamma[2]
                        y_proj = x0_gamma[1] + (z - x0_gamma[2])*p_gamma[1]/p_gamma[2]
                        projdist = math.sqrt((x-x_proj)**2 + (y-y_proj)**2)
                        rMean_gammaProj += projdist*energy
