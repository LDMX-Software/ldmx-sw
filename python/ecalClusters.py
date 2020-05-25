"""Configuration for the EcalClusterProducer

Examples
--------
>>> from LDMX.Ecal.ecalClusters import ecalClusters
>>> p.sequence.append( ecalClusters )
"""

import sys
import os

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

ecalClusters = ldmxcfg.Producer("ecalClusters", "ldmx::EcalClusterProducer")

# Cluster weight cutoff in appropriate units
ecalClusters.parameters["cutoff"] = 10.0 

# Seed threshold for clustering
ecalClusters.parameters["seedThreshold"] = 100.0 # MeV

# Pass name for ecal digis
ecalClusters.parameters["digisPassName"] = "recon"

# Name of the algo to save to the root file 
ecalClusters.parameters["algoName"] = "MyClusterAlgo"

# Name of the cluster collection to make
ecalClusters.parameters["clusterCollName"] = "ecalClusters"

# Name of the cluster algo collection to make
ecalClusters.parameters["algoCollName"] = "ClusterAlgoResult"
