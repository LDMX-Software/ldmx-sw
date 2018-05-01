#!/usr/bin/python

import sys
import os

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# Setup producers with default templates
from LDMX.EventProc.ecalDigis import ecalDigis
from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.EventProc.simpleTrigger import simpleTrigger
from LDMX.EventProc.trackerHitKiller import trackerHitKiller

p = ldmxcfg.Process("recon")
p.libraries.append("libEventProc.so")

# Load the PN re-weighting processor
pnWeight = ldmxcfg.Producer("pn_reweight", "ldmx::PnWeightProcessor")
pnWeight.parameters["w_threshold"] = 1150.
pnWeight.parameters["theta_threshold"] = 100.

ecalVeto = ldmxcfg.Producer("ecalVeto", "ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 34
ecalVeto.parameters["do_bdt"] = 1
ecalVeto.parameters["fid_bdt_file"] = "fid_bdt.pkl"
ecalVeto.parameters["nf_bdt_files"] = ["p001_nf_bdt.pkl", "p01_nf_bdt.pkl", "p1_nf_bdt.pkl", "p0_nf_bdt.pkl"]
ecalVeto.parameters["cellxy_file"] = "cellxy.txt"
ecalVeto.parameters["disc_cut"] = [0.96, 0.95, 0.95, 0.95, 0.95]
#Set to 1 to drop fid or non-fid events respectively (Must listen and drop by default).
ecalVeto.parameters["drop_fid_nf"] = [0, 0]

hcalVeto = ldmxcfg.Producer("hcalVeto", "ldmx::HcalVetoProcessor")
hcalVeto.parameters["pe_threshold"] = 8.0

simpleTrigger.parameters["threshold"]   = 1500.0 # MeV 
simpleTrigger.parameters["end_layer"]   = 20 

findable_track = ldmxcfg.Producer("findable", "ldmx::FindableTrackProcessor")

ecalSimHitSort = ldmxcfg.Producer("ecalSimHitSort", "ldmx::SimHitSortProcessor")
ecalSimHitSort.parameters["simHitCollection"]="EcalSimHits"
ecalSimHitSort.parameters["outputCollection"]="SortedEcalSimHits"
hcalSimHitSort = ldmxcfg.Producer("hcalSimHitSort", "ldmx::SimHitSortProcessor")
hcalSimHitSort.parameters["simHitCollection"]="HcalSimHits"
hcalSimHitSort.parameters["outputCollection"]="SortedHcalSimHits"

p.sequence=[ecalDigis, hcalDigis, simpleTrigger, ecalVeto, hcalVeto, trackerHitKiller, findable_track, pnWeight, ecalSimHitSort, hcalSimHitSort]

# Default to dropping all events
p.skimDefaultIsDrop()

# Use output of trigger module to decide what to keep
p.skimConsider("simpleTrigger")

p.inputFiles = [sys.argv[1]]
p.outputFiles = ["ldmx_skim_digi_events.root"]

p.printMe()
