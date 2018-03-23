#!/usr/bin/python

import sys
import os

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

clusterAnalyzer = ldmxcfg.Analyzer("clusterAnalyzer", "ldmx::ClusterAnalyzer")

# Which pass of the cluster collection to grab
clusterAnalyzer.parameters["passName"] = "recon"

# Name of the cluster collection to grab 
clusterAnalyzer.parameters["clusterCollName"] = "ecalClusters"

# Name of the cluster algo collection to grab 
clusterAnalyzer.parameters["algoCollName"] = "ClusterAlgoResult"

# Minimum number of hits of a cluster
clusterAnalyzer.parameters["minHits"] = 2
