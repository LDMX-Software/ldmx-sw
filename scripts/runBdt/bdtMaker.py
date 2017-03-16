#!/usr/bin/python
import argparse
import importlib
import ROOT as r
import os
import math
import sys
from array import array
from optparse import OptionParser
import numpy as np
import xgboost as xgb
import pickle as pkl
sys.path.insert(0, '../')
np.random.seed(0)
from sklearn import metrics,preprocessing
from math import sqrt
######################################################################
class sampleContainer:
    def __init__(self, fn,maxEvts,trainFrac,isBkg,dataMethod = False):
        print "Initializing Container!"
        self.tin = r.TChain("LDMX_Events");
        self.tin.Add(fn);

        self.maxEvts   = maxEvts;
        self.trainFrac = trainFrac;
        self.isBkg = isBkg;
        self.dataMethod = dataMethod;

        self.evHeader = r.ldmx.EventHeader();
        self.trigRes = r.TClonesArray('ldmx::TriggerResult');
        self.simParticles = r.TClonesArray('ldmx::SimParticle');
        self.ecalHits = r.TClonesArray('ldmx::EcalHit');
        self.hcalHits = r.TClonesArray('ldmx::HcalHit');
        self.ecalSimHits = r.TClonesArray('ldmx::SimCalorimeterHit');
        self.hcalSimHits = r.TClonesArray('ldmx::SimCalorimeterHit');
        self.ecalVetoRes = r.TClonesArray('ldmx::EcalVetoResult');
        self.trackRes    = r.TClonesArray('ldmx::FindableTrackResult');
        self.tin.SetBranchAddress("EventHeader",  r.AddressOf( self.evHeader ));
        self.tin.SetBranchAddress("Trigger_recon",  r.AddressOf( self.trigRes ));
        self.tin.SetBranchAddress("SimParticles_sim",  r.AddressOf( self.simParticles ));
        self.tin.SetBranchAddress("EcalSimHits_sim",  r.AddressOf( self.ecalSimHits ));
        self.tin.SetBranchAddress("HcalSimHits_sim",  r.AddressOf( self.hcalSimHits ));
        self.tin.SetBranchAddress("hcalDigis_recon",  r.AddressOf( self.hcalHits ));
        self.tin.SetBranchAddress("ecalDigis_recon",  r.AddressOf( self.ecalHits ));
        self.tin.SetBranchAddress("EcalVeto_recon",  r.AddressOf( self.ecalVetoRes ));
        self.tin.SetBranchAddress("FindableTracks_recon",  r.AddressOf( self.trackRes ));
    def root2PyEvents(self):
        self.events =  []
        for event in xrange(min(self.maxEvts,self.tin.GetEntries())):
            evt = []
            self.tin.GetEntry(event)
            #for entry in (self.ecalVetoRes[0].getEcalLayerEdepReadout())[0:33]:
            #    evt.append(entry)

            evt.append(self.ecalVetoRes[0].getNReadoutHits());
            evt.append(self.ecalVetoRes[0].getNLooseIsoHits());
            evt.append(self.ecalVetoRes[0].getNTightIsoHits());
            evt.append(self.ecalVetoRes[0].nLooseMipTracks());
            evt.append(self.ecalVetoRes[0].nMediumMipTracks());
            evt.append(self.ecalVetoRes[0].nTightMipTracks());
            evt.append(self.ecalVetoRes[0].getSummedDet());
            evt.append(self.ecalVetoRes[0].getSummedOuter());
            evt.append(self.ecalVetoRes[0].getBackSummedDep());
            evt.append(self.ecalVetoRes[0].getSummedLooseIso());
            evt.append(self.ecalVetoRes[0].getMaxLooseIsoDep());
            evt.append(self.ecalVetoRes[0].getSummedTightIso());
            evt.append(self.ecalVetoRes[0].getMaxTightIsoDep());
            evt.append(self.ecalVetoRes[0].getMaxCellDep());
            evt.append(self.ecalVetoRes[0].getShowerRMS());
            maxLen = 0;
            summedDep = 0;
            for track in self.ecalVetoRes[0].getLooseMipTracks():
                if (track.first > maxLen):
                    maxLen = track.first;
                summedDep += track.second;

            evt.append(maxLen);
            evt.append(summedDep);

            maxLen = 0;
            summedDep = 0;
            for track in self.ecalVetoRes[0].getMediumMipTracks():
                if (track.first > maxLen):
                    maxLen = track.first;
                summedDep += track.second;

            evt.append(maxLen);
            evt.append(summedDep);


            maxLen = 0;
            summedDep = 0;
            for track in self.ecalVetoRes[0].getTightMipTracks():
                if (track.first > maxLen):
                    maxLen = track.first;
                summedDep += track.second;

            evt.append(maxLen);
            evt.append(summedDep);
            if (self.isBkg and self.dataMethod):
                pe = 0
                for hit in self.hcalHits:
                    pe += hit.getPE();
                evt.append(pe)
            self.events.append(evt)
            if (len(self.events)%1000 == 0 and len(self.events) > 0):
                print 'The shape of events = (%s,%s) ' % (len(self.events),len(self.events[0]))
        self.events = np.array(self.events)
        print self.events.shape
        #if (self.isBkg):
        #    self.events = self.events[:,range(len(self.events[0])-1)][self.events[:,-1] > 8]


    def constructTrainAndTest(self):
        if (self.isBkg == False):
          np.random.shuffle(self.events)
        self.train_x = self.events[0:int(len(self.events)*self.trainFrac)]
        self.test_x  = self.events[int(len(self.events)*self.trainFrac):]
        if (self.isBkg and self.dataMethod):
          self.train_x = self.events[self.events[:,-1] > 8][:,range(len(self.events[0])-1)]
          self.test_x  = self.events[self.events[:,-1] < 8][:,range(len(self.events[0])-1)]


        self.train_y = np.zeros(len(self.train_x)) + (self.isBkg == False)
        self.test_y  = np.zeros(len(self.test_x)) + (self.isBkg == False)

class mergedContainer:
    def __init__(self, sigContainer,bkgContainer):
        print 'Signal Training shape = (%s,%s)' % (sigContainer.train_x.shape)
        print 'Bkg Training shape = (%s,%s)' % (bkgContainer.train_x.shape)
        print 'Signal Testing shape = (%s,%s)' % (sigContainer.test_x.shape)
        print 'Bkg Testing shape = (%s,%s)' % (bkgContainer.test_x.shape)
        print 'Sig/Bkg Training target sum = (%s,%s)' % (np.sum(sigContainer.train_y),np.sum(bkgContainer.train_y))
        print 'Signal/Bkg Training wgts = (%s,%s)' % \
          (np.zeros(len(sigContainer.train_y)) + 1,np.zeros(len(bkgContainer.train_y)) + 1. * float(len(sigContainer.train_y))/float(len(bkgContainer.train_y)))


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
    parser.add_option('--bkg_file', dest='bkg_file', default='bkg.root', help='name of signal file')
    parser.add_option('--sig_file', dest='sig_file', default='signal.root', help='name of signal file')
    parser.add_option('--train_frac', dest='train_frac',  default=.75, help='Fraction of events to use for training')
    parser.add_option('--max_evt', dest='max_evt',type="int",  default=1000, help='Max Events to load')
    parser.add_option('--out_name', dest='out_name',  default='bdt', help='Output Pickle Name')
    parser.add_option('--swdir', dest='swdir',  default='ldmx-install', help='ldmx-sw build directory')
    parser.add_option("--data_train", dest='data_train', action="store_true",  default=False)
    (options, args) = parser.parse_args()
    print 'You set data_train_method = %s ' % (options.data_train)

    print 'Loading library file from %s' % (options.swdir+"/lib/libEvent.so")
    r.gSystem.Load(options.swdir+"/lib/libEvent.so")
    print 'Loading sig_file = %s' % (options.sig_file)
    sigContainer = sampleContainer(options.sig_file,options.max_evt,options.train_frac,False,options.data_train)
    sigContainer.root2PyEvents()
    sigContainer.constructTrainAndTest()
    print 'Loading bkg_file = %s' % (options.bkg_file)
    bkgContainer = sampleContainer(options.bkg_file,int(1.5*options.max_evt),options.train_frac,True,options.data_train)
    bkgContainer.root2PyEvents()
    bkgContainer.constructTrainAndTest()

    eventContainer = mergedContainer(sigContainer,bkgContainer)
    print eventContainer.train_x.shape
    #print eventContainer.train_x[eventContainer.train_x[:,0] == np.nan]
    #print eventContainer.train_x[eventContainer.train_x[:,0] == np.nan][0]

    params     = {"objective": "binary:logistic",
               "eta": 0.1,
               "max_depth": 6,
               "min_child_weight": 20,
               "silent": 1,
               "subsample": .9,
               "colsample_bytree": .85,
               "eval_metric": 'auc',
               "seed": 1,
               "nthread": 1,
               "verbosity": 1,
               "early_stopping_rounds" : 10}
    num_trees  = 1000
    evallist  = [(eventContainer.dtest,'eval'), (eventContainer.dtrain,'train')]
    gbm       = xgb.train(params, eventContainer.dtrain, num_trees,evallist)

    output = open(options.out_name +'.pkl', 'wb')
    pkl.dump(gbm, output)

    # calculate the fpr and tpr for all thresholds of the classification
    probs = gbm.predict(eventContainer.dtest)
    preds = probs
    fpr, tpr, threshold = metrics.roc_curve(eventContainer.test_y, preds)
    roc_auc = metrics.auc(fpr, tpr)
    print 'The auc = %s' % (roc_auc)
    np.savetxt(options.out_name +'_result.txt',np.c_[fpr,tpr,threshold])

    percentiles_vector_dict = {}
    percentile_cut_dict = {}
    for var in range(len(eventContainer.train_x[0])):
        tmp = []
        for percentile in np.linspace(10,100,20):
            tmp.append(np.percentile(eventContainer.train_x[:,var],percentile))
        percentiles_vector_dict[str(var)] = tmp
    while True:
        if (len(percentiles_vector_dict.keys()) == 0): break;
        cutVar,cutVal,cutSoverB = -1,-1,-1;
        for key in percentiles_vector_dict.keys():
            for percentile_cut in percentiles_vector_dict[key]:
                S = np.sum(sigContainer.train_x[:,int(key)] < percentile_cut)
                B = np.sum(bkgContainer.train_x[:,int(key)] < percentile_cut)
                if S/sqrt(S+B) > cutSoverB:
                    cutSoverB = S/sqrt(S+B)
                    cutVar = key
                    cutVal = percentile_cut
        print len(percentiles_vector_dict.keys()),cutVar,cutVal,cutSoverB
        percentile_cut_dict[cutVar] = cutVal
        del percentiles_vector_dict[cutVar]
        print percentile_cut_dict
    percentile_cut_vector = [percentile_cut_dict[str(ele)] for ele in range(len(percentile_cut_dict.keys()))]
    print percentile_cut_vector
    for var,cut in enumerate(percentile_cut_vector):
        sigContainer.test_x[:,var] = sigContainer.test_x[:,var] > cut
        bkgContainer.test_x[:,var] = bkgContainer.test_x[:,var] > cut
    print sigContainer.test_x.shape
    print np.apply_along_axis(np.sum,axis=1,arr=sigContainer.test_x).shape
    passVetoSig  = [(ele == 0) for ele in np.apply_along_axis(np.sum,axis=1,arr=sigContainer.test_x)]
    passVetoBkg  = [(ele == 0) for ele in np.apply_along_axis(np.sum,axis=1,arr=bkgContainer.test_x)]
    print np.sum(passVetoSig),len(passVetoSig)
    print np.sum(passVetoBkg),len(passVetoBkg)
