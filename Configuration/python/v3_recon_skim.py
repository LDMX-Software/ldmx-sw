#!/usr/bin/python

import sys
import os

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# Setup producers with default templates
from LDMX.EventProc.ecalDigis import ecalDigis
from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.EventProc.simpleTrigger import simpleTrigger 

p = ldmxcfg.Process("skim")
p.libraries.append("libEventProc.so")

# Load the PN re-weighting processor
pnWeight = ldmxcfg.Producer("pn_reweight", "ldmx::PnWeightProcessor")

# Set the W threshold above which an event will be re-weighted.  For the 
# definition of W, see the processor.
pnWeight.parameters["w_threshold"] = 1200.
pnWeight.parameters["theta_threshold"] = 100.

ecalVeto = ldmxcfg.Producer("ecalVeto", "ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 34
ecalVeto.parameters["do_bdt"] = 1
ecalVeto.parameters["bdt_file"] = "cal_bdt.pkl"
ecalVeto.parameters["disc_cut"] = 0.999672

hcalVeto = ldmxcfg.Producer("hcalVeto", "ldmx::HcalVetoProcessor")
hcalVeto.parameters["pe_threshold"] = 8.0

simpleTrigger.parameters["threshold"]   = 1500.0 # MeV 
simpleTrigger.parameters["end_layer"]   = 20 

findable_track = ldmxcfg.Producer("findable", "ldmx::FindableTrackProcessor")

p.sequence=[ecalDigis, hcalDigis, simpleTrigger, ecalVeto, hcalVeto, findable_track, pnWeight]

# Default to dropping all events
p.skimDefaultIsDrop()

# Use output of trigger module to decide what to keep
p.skimConsider("simpleTrigger")

p.inputFiles = [sys.argv[1]]
p.outputFiles = ["ldmx_skim_digi_events.root"]

p.printMe()
