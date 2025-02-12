"""Macro for training visibles search BDT

   author: Tyler Horoho & Kieran Wall (UVA)

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

                        closestpoint = 9999
                        for hit2 in HcalRecHits :
                            if abs(z - hit2.getZPos()) == 0 :
                                sepx = math.sqrt((x-hit2.getXPos())**2)
                                sepy = math.sqrt((y-hit2.getYPos())**2)
                                if sepx > 0 and sepx%50 == 0 :
                                    if sepx < closestpoint :
                                        closestpoint = spex
                                elif sepy > 0 and sepy%50 == 0 :
                                    if sepy < closestpoint :
                                        closestpoint = sepy
                        if closestpoint > 50:
                            isohits += 1
                            isoE += energy

                xmean /= Ehcal
                ymean /= Ehcal
                zmean /= Ehcal
                rmean /= Ehcal

                rMean_gammaProj /= Ehcal

                for hit in HcalRecHits :
                    if hit.getZPos() >= 870 :
                        x = hit.getXPos()
                        y = hit.getYPos()
                        z = hit.getZPos()
                        energy = hit.getEnergy()

                        xstd += energy*(x-xmean)**2
                        ystd += energy*(y-ymean)**2
                        zstd += energy*(z-zmean)**2

                xstd = math.sqrt(xstd/Ehcal)
                ystd = math.sqrt(ystd/Ehcal)
                zstd = math.sqrt(zstd/Ehcal)

                # Fill event with features to train BDT

                evt.append(len(layersHit))

                evt.append(xstd)
                evt.append(ystd)
                evt.append(zstd)

                evt.append(xmean)
                evt.append(ymean)
                evt.append(rmean)

                evt.append(isohits)
                evt.append(isoE)
                evt.append(hits)

                evt.append(Ehcal)

                evt.append(rMean_gammaProj)

                self.events.append(evt)
                evtcount += 1

                if evtcount > self.maxEvts:
                    break
            if evtcount > self.maxEvts:
                break

        print("Initial Event Shape:", np.shape(self.events))
        new_idx = np.random.permutation(np.arange(np.shape(self.events)[0]))
        self.events = np.array(self.events)
        np.take(self.events, new_idx, axis=0, out=self.events)
        print("Final Event Shape:", np.shape(self.events))

    def constructTrainAndTest(self) :

        self.train_x = self.events[0:int(len(self.events)*self.trainFrac)]
        self.test_x  = self.events[int(len(self.events)*self.trainFrac):]

        self.train_y = np.zeros(len(self.train_x)) + (self.isBkg == False)
        self.test_y  = np.zeros(len(self.test_x)) + (self.isBkg == False)

class mergedContainer :
    def __init__(self, sigContainer, bkgContainer) :
        self.train_x = np.vstack((sigContainer.train_x, bkgContainer.train_x))
        self.train_y = np.append(sigContainer.train_y, bkgContainer.train_y)

        self.train_x[np.isnan(self.train_x)] = 0.000
        self.train_y[np.isnan(self.train_y)] = 0.000

        self.test_x = np.vstack((sigContainer.test_x, bkgContainer.test_x))
        self.test_y = np.append(sigContainer.test_y, bkgContainer.test_y)

        self.dtrain = xgb.DMatrix(self.train_x, self.train_y, weight = self.getEventWeights(sigContainer.train_y, bkgContainer.train_y))
        self.dtest = xgb.DMatrix(self.test_x, self.test_y)


    def getEventWeights(self, sig, bkg) :
        sigWgt = np.zeros(len(sig)) + 1
        bkgWgt = np.zeros(len(bkg)) + 1.*float(len(sig))/float(len(bkg))
        return np.append(sigWgt, bkgWgt)

if __name__ == '__main__' :

    parser = OptionParser()

    parser.add_option('--seed', dest='seed', type='int', default=4, help='Numpy random seed.')
    parser.add_option('--train_frac', dest='train_frac', default=0.9999, help='Fraction of events to use for training')
    parser.add_option('--max_evt', dest='max_evt', type='int', default=3000000, help='Max Events to load')
    parser.add_option('--out_dir', dest='out_dir', default='~/', help='Output directory')
    parser.add_option('--out_name', dest='out_name', default='bdt', help='Output Pickle Name')
    parser.add_option('--eta', dest='eta', type='float', default=0.023, help='Learning Rate')
    parser.add_option('--tree_number', dest='tree_number', type='int', default=1000, help='Tree Number')
    parser.add_option('--depth', dest='depth', type='int', default=6, help='Max Tree Depth')
    parser.add_option('--bkg_dir', dest='bkg_dir', default='~/', help='name of background file directory')
    parser.add_option('--sig_dir', dest='sig_dir', default='~/', help='name of signal file directory')

    (options, args) = parser.parse_args()

    np.random.seed(options.seed)

    adds = 0
    Check = True
    while Check :
        if not os.path.exists(options.out_name+'_'+str(adds)) :
            try :
                os.makedirs(options.out_name+'_'+str(adds))
                Check = False
            except :
                Check = True
            else :
                adds += 1

    print("Random seed is", options.seed)
    print("Tree number:", options.tree_number)
    print("Max tree depth:", options.depth)
    print("Eta:", options.eta)
    print("Max training events:", options.max_evt)
    print("Training fraction:", options.train_frac)

    print("Loading signal files from", options.sig_dir)
    sigContainer = sampleContainer(options.sig_dir, options.max_evt, options.train_frac, False)
    sigContainer.constructTrainAndTest()

    print("Loading background files from", options.bkg_dir)
    bkgContainer = sampleContainer(options.bkg_dir, options.max_evt, options.train_frac, True)
    bkgContainer.constructTrainAndTest()

    mergedContainer = mergedContainer(sigContainer, bkgContainer)

    params = {"objective": "binary:logistic",
              "eta": options.eta,
              "max_depth": options.depth,
              "min_child_weight": 20,
              "silent": 0,
              "subsample": 0.9,
              "colsample_bytree": 0.85,
              "eval_metric": 'auc',
              "seed": 1,
              "nthread": 1,
              "verbosity": 1,
              "early_stopping_rounds": 10}

    evallist = [(eventContainer.dtest, 'eval'), (eventContainer.dtrain, 'train')]
    gbm = xbg.train(params, eventContainer.dtrain, options.tree_number, evallist)

    # Eval metrics for testing data
    preds = gbm.predict(mergedContainer.dtest)
    fpr, tpr, threshold = metrics.roc_curve(mergedContainer.test_y, preds)
    roc_auc = metrics.auc(fpr, tpr)
    print("Final validation AUC:", roc_auc)

    # Eval metrics for training data
    pred_train = gbm.predict(eventContainer.dtrain)
    fpr_t, tpr_t, threshold_t = metrics.roc_curve(eventContainer.train_y, preds_train)

    np.savetxt(options.out_name+'_'+str(adds)+'/'+options.out_name+'_'+str(adds)+'_training_threetuples.txt', np.c_[fpr_t, tpr_t, threshold_t])
    np.savetxt(options.out_name+'_'+str(adds)+'/'+options.out_name+'_'+str(adds)+'_validation_preds.txt', np.c_[preds, eventContainer.test_y])
    np.savetxt(options.out_name+'_'+str(adds)+'/'+options.out_name+'_'+str(adds)+'_validation_threetuples.txt', np.c_[fpr, tpr, threshold])
    output = open(options.out_name+'_'+str(adds)+'/'+options.out_name+'_'+str(adds)+'_weights'+'.pkl', 'wb')
    pkl.dump(gbm, output)

    xgb.plot_importance(gbm)
    plt.pyplot.savefig(options.out_name+'_'+str(adds)+'/'+options.out_name+'_'+str(adds)+'_fimportance.png', dpi=500, bbox_inches='tight', pad_inches=0.5)

    print("Files saved in: ", options.out_name+'_'+str(adds))
