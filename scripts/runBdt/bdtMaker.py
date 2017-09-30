#!/usr/bin/python
import argparse
import importlib
import os
import math
import sys
import random
import ROOT as r
import matplotlib as plt
import xgboost as xgb
import pickle as pkl
import numpy as np

plt.use('Agg')
from collections import Counter
from array import array
from optparse import OptionParser
sys.path.insert(0, '../')
from sklearn import metrics

####################################################################################
class sampleContainer:
    def __init__(self, fn,maxEvts,trainFrac,isBkg,iseBkg,iseSig):
        print "Initializing Container!"
        self.tin = r.TChain("LDMX_Events")
        self.tin.Add(fn)

        self.maxEvts   = maxEvts
        self.trainFrac = trainFrac
        self.isBkg = isBkg
	self.iseBkg = iseBkg
	self.iseSig = iseSig

        self.evHeader = r.ldmx.EventHeader()
        self.simParticles = r.TClonesArray('ldmx::SimParticle')
        self.ecalHits = r.TClonesArray('ldmx::EcalHit')
        self.hcalHits = r.TClonesArray('ldmx::HcalHit')
        self.ecalSimHits = r.TClonesArray('ldmx::SimCalorimeterHit')
        self.hcalSimHits = r.TClonesArray('ldmx::SimCalorimeterHit')
        self.ecalVetoRes = r.TClonesArray('ldmx::EcalVetoResult')

        self.tin.SetBranchAddress("EventHeader",  r.AddressOf( self.evHeader ))
        self.tin.SetBranchAddress("SimParticles_sim",  r.AddressOf( self.simParticles ))
        self.tin.SetBranchAddress("EcalSimHits_sim",  r.AddressOf( self.ecalSimHits ))
        self.tin.SetBranchAddress("ecalDigis_recon",  r.AddressOf( self.ecalHits ))
        self.tin.SetBranchAddress("EcalVeto_recon",  r.AddressOf( self.ecalVetoRes ))


    def root2PyEvents(self):
        self.events =  []
        for event in xrange(min(self.maxEvts,self.tin.GetEntries())):
            evt = []
            self.tin.GetEntry(event)
            

################################### Features #######################################

            evt.append(self.ecalVetoRes[0].getNReadoutHits())

            evt.append(self.ecalVetoRes[0].getSummedDet())

            evt.append(self.ecalVetoRes[0].getSummedLooseIso())

            evt.append(self.ecalVetoRes[0].getSummedTightIso())

            evt.append(self.ecalVetoRes[0].getMaxCellDep())

            evt.append(self.ecalVetoRes[0].getShowerRMS())
            
            evt.append(self.ecalVetoRes[0].getXStd())

            evt.append(self.ecalVetoRes[0].getYStd())

            evt.append(self.ecalVetoRes[0].getXMean())

            evt.append(self.ecalVetoRes[0].getYMean())

            evt.append(self.ecalVetoRes[0].getAvgLayerHit())

            evt.append(self.ecalVetoRes[0].getDeepestLayerHit())

            evt.append(self.ecalVetoRes[0].getStdLayerHit())

######################################################################################
            self.events.append(evt)

            if (len(self.events)%10000 == 0 and len(self.events) > 0):
                print 'The shape of events = ', np.shape(self.events)


        new_idx=np.random.permutation(np.arange(np.shape(self.events)[0]))
        self.events = np.array(self.events)
        np.take(self.events, new_idx, axis=0, out=self.events)
        print "Final Event Shape", np.shape(self.events)

    def constructTrainAndTest(self):
        self.train_x = self.events[0:int(len(self.events)*self.trainFrac)]
        self.test_x  = self.events[int(len(self.events)*self.trainFrac):]
        
        self.train_y = np.zeros(len(self.train_x)) + (self.isBkg == False)
        self.test_y  = np.zeros(len(self.test_x)) + (self.isBkg == False)

class mergedContainer:
    def __init__(self, sigContainer,bkgContainer):
        self.train_x = np.vstack((sigContainer.train_x,bkgContainer.train_x))
        self.train_y = np.append(sigContainer.train_y,bkgContainer.train_y)
        
        self.train_x[np.isnan(self.train_x)] = 0.000
        self.train_y[np.isnan(self.train_y)] = 0.000
        
        self.test_x  = np.vstack((sigContainer.test_x,bkgContainer.test_x))
        self.test_y  = np.append(sigContainer.test_y,bkgContainer.test_y)
        
        self.dtrain = xgb.DMatrix(self.train_x,self.train_y,\
                                    weight = self.getEventWeights(sigContainer.train_y,bkgContainer.train_y))
        self.dtest  = xgb.DMatrix(self.test_x,self.test_y)
    


    #Class for re-weighting bkg to signal
    def getEventWeights(self,sig,bkg):
        sigWgt = np.zeros(len(sig)) + 1
        bkgWgt = np.zeros(len(bkg)) + 1. * float(len(sig))/float(len(bkg))
        return np.append(sigWgt,bkgWgt)

if __name__ == "__main__":
    
    parser = OptionParser()


    parser.add_option('--seed', dest='seed',type="int",  default=1, help='Numpy random seed.')
    parser.add_option('--train_frac', dest='train_frac',  default=.5, help='Fraction of events to use for training')
    parser.add_option('--max_evt', dest='max_evt',type="int",  default=960306, help='Max Events to load')
    parser.add_option('--out_name', dest='out_name',  default='bdt', help='Output Pickle Name')
    parser.add_option('--swdir', dest='swdir',  default='../../install', help='ldmx-sw build directory')
    parser.add_option('--eta', dest='eta',type="float",  default=0.023, help='Learning Rate')
    parser.add_option('--tree_number', dest='tree_number',type="int",  default=1000, help='Tree Number')
    parser.add_option('--depth', dest='depth',type="int",  default=10, help='Max Tree Depth')
    parser.add_option('--bkg_file', dest='bkg_file', default='/nfs/slac/g/ldmx/data/mc/bdt_data/new_recon_bkg.root', help='name of background file')
    parser.add_option('--sig_file', dest='sig_file', default='/nfs/slac/g/ldmx/data/mc/bdt_data/new_recon_sig.root', help='name of signal file')
  

    (options, args) = parser.parse_args()

    np.random.seed(options.seed)
    
    adds=0
    Check=True
    while Check:
        if not os.path.exists(options.out_name+'_'+str(adds)):
	    try:
                os.makedirs(options.out_name+'_'+str(adds))
	        Check=False
	    except:
	        Check=True
        else:
            adds+=1


    print "Random seed is = %s" % (options.seed)
    print "You set max_evt = %s" % (options.max_evt)
    print "You set train frac = %s" % (options.train_frac)
    print "You set tree number = %s" % (options.tree_number)
    print "You set max tree depth = %s" % (options.depth)
    print "You set eta = %s" % (options.eta)

    print "Loading library file from %s" % (options.swdir+"/lib/libEvent.so")
    r.gSystem.Load(options.swdir+"/lib/libEvent.so")


    print 'Loading sig_file = %s' % (options.sig_file)
    sigContainer = sampleContainer(options.sig_file,options.max_evt,options.train_frac,False,False,False)
    sigContainer.root2PyEvents()
    sigContainer.constructTrainAndTest()

    print 'Loading bkg_file = %s' % (options.bkg_file)
    bkgContainer = sampleContainer(options.bkg_file,options.max_evt,options.train_frac,True,False,False)
    bkgContainer.root2PyEvents()
    bkgContainer.constructTrainAndTest()

    eventContainer = mergedContainer(sigContainer,bkgContainer)

    params     = {"objective": "binary:logistic",
                "eta": options.eta,
                "max_depth": options.depth,
                "min_child_weight": 20,
                "silent": 1,
                "subsample": .9,
                "colsample_bytree": .85,
                "eval_metric": 'auc',
                "seed": 1,
                "nthread": 1,
                "verbosity": 1,
                "early_stopping_rounds" : 10}

    num_trees  = options.tree_number
    evallist  = [(eventContainer.dtest,'eval'), (eventContainer.dtrain,'train')]
    gbm       = xgb.train(params, eventContainer.dtrain, num_trees,evallist)


    preds = gbm.predict(eventContainer.dtest)
    fpr, tpr, threshold = metrics.roc_curve(eventContainer.test_y, preds)
    roc_auc = metrics.auc(fpr, tpr)
    print 'Final Validation AUC = %s' % (roc_auc)
    np.savetxt(options.out_name+'_'+str(adds)+"/"+options.out_name+'_'+str(adds)+'_validation_preds.txt',preds)
    np.savetxt(options.out_name+'_'+str(adds)+"/"+options.out_name+'_'+str(adds)+'_validation_threetuples.txt',np.c_[fpr,tpr,threshold])
    output = open(options.out_name+'_'+str(adds)+"/"+options.out_name+'_'+str(adds)+"_weights"+'.pkl', 'wb')
    pkl.dump(gbm, output)

    xgb.plot_importance(gbm)
    plt.pyplot.savefig(options.out_name+'_'+str(adds)+"/"+options.out_name+'_'+str(adds)+'_fimportance.png', dpi=500, bbox_inches='tight', pad_inches=0.5)

    print "Files saved in: ", options.out_name+'_'+str(adds)
