#!/usr/bin/python

import sys

# Load the configuration package
from LDMX.Framework import ldmxcfg

# Set the process name
p=ldmxcfg.Process("recon")

# Load the library that contains the Ecal veto processor
p.libraries.append("libEventProc.so")

# Configure the Ecal veto processor
ecalVeto = ldmxcfg.Producer("ecalVeto", "ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 34
ecalVeto.parameters["do_bdt"] = 1
ecalVeto.parameters["fid_bdt_file"] = "fid_bdt.pkl"
ecalVeto.parameters["nf_bdt_files"] = ["p001_nf_bdt.pkl", "p01_nf_bdt.pkl", "p1_nf_bdt.pkl", "p0_nf_bdt.pkl"]
ecalVeto.parameters["cellxy_file"] = "cellxy.txt"
ecalVeto.parameters["disc_cut"] = [0.96, 0.95, 0.95, 0.95, 0.95]
#Set to 1 to drop fid or non-fid events respectively (Must listen and drop by default).
ecalVeto.parameters["drop_fid_nf"] = [0, 0]

# Add the processor to the processing chain
p.sequence=[ecalVeto]

# Default to dropping all events
p.skimDefaultIsDrop()

# Use the output of the Ecal veto processor to decide what events should be 
# dropped
p.skimConsider("ecalVeto")

# Set the input and output files to use
p.inputFiles=[sys.argv[1]]
p.outputFiles=[sys.argv[2]]
