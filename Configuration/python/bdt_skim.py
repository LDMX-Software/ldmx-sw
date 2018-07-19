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
ecalVeto.parameters["bdt_file"] = "fid_bdt.pkl"
ecalVeto.parameters["cellxy_file"] = "cellxy.txt"
ecalVeto.parameters["disc_cut"] = 0.95

# Configure the Non-Fiducial Ecal veto processor
NonFidecalVeto = ldmxcfg.Producer("NonFidecalVeto", "ldmx::NonFidEcalVetoProcessor")
NonFidecalVeto.parameters["num_ecal_layers"] = 34
NonFidecalVeto.parameters["do_bdt"] = 1
#Files in order of increasing mass
NonFidecalVeto.parameters["nf_bdt_files"] = ["p001_nf_bdt.pkl", "p01_nf_bdt.pkl", "p1_nf_bdt.pkl", "p0_nf_bdt.pkl"]
NonFidecalVeto.parameters["cellxy_file"] = "cellxy.txt"
#Disc cuts in order of increasing mass
NonFidecalVeto.parameters["disc_cut"] = [0.99, 0.95, 0.94, 0.94]

# Add the processor to the processing chain
# If you are dropping fiducial or non-fiducial events you can delete one of these.
p.sequence=[ecalVeto, NonFidecalVeto]

# Default to dropping all events
p.skimDefaultIsDrop()

# Use the output of the Ecal veto processor to decide what events should be 
# dropped
# This option will also drop non-fiducial events. To drop fiducial events change this to 
# NonFidecalVeto
p.skimConsider("ecalVeto")

# Set the input and output files to use
p.inputFiles=[sys.argv[1]]
p.outputFiles=[sys.argv[2]]
