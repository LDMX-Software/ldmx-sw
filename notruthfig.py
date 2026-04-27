#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Apr 14 21:58:25 2026

@author: kvarn
"""

#digis, then clusters, then makes clusters.txt

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('nontruthclusters')
p.input_files =  ["SimSamples.root"]
p.output_files = ["AllSamples.root"]
#an additional output file called clusters.txt will be created as well

from LDMX.TrigScint.trig_scint import TrigScintDigiProducer

pad_num = 1

digis = [TrigScintDigiProducer.pad1(),
         TrigScintDigiProducer.pad2(),
         TrigScintDigiProducer.pad3()
         ]

for digi in digis:
    digi.input_collection = f"TriggerPad{pad_num}SimHits"
    pad_num+=1
    
from LDMX.TrigScint.trig_scint import TrigScintClusterProducer
    
clusters = [
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        ]

for cluster, digi in zip(clusters, digis):
    cluster.input_collection = digi.output_collection
    cluster.ampl_weighting = False
    cluster.clustering_threshold = 3.0

CVA= ldmxcfg.Analyzer.from_file('ClusterViewerAnalyzer.cxx', 
     needs = ['TrigScint_Event', 'SimCore_Event'])

CVA.pass_name = "nontruthclusters"

p.sequence = [
             #*truth_hits,
              *digis,
              *clusters, 
              CVA
              ]

    
