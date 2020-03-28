#!/usr/bin/python

import sys
import os

# We need the ldmx configuration package to construct the processor objects
from LDMX.Framework import ldmxcfg

# Setup producers with default templates
from LDMX.Ecal.ecalSim2Rec import ecalSim2Rec
from LDMX.EventProc.ecalVeto import ecalVeto
from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.EventProc.hcalVeto import hcalVeto
from LDMX.EventProc.simpleTrigger import simpleTrigger
from LDMX.EventProc.trackerHitKiller import trackerHitKiller

# Define the process, which must have a name which identifies this
# processing pass ("pass name").
p = ldmxcfg.Process("recon")

# Currently, we need to explicitly identify plugin libraries which should be
# loaded.  In future, we do not expect this be necessary
p.libraries.append("libEventProc.so")
p.libraries.append("libEcal.so")

# Create a processor to determine how many findable tracks are in an event.
findableTrack = ldmxcfg.Producer("findable", "ldmx::FindableTrackProcessor")

# Create a processor to veto events with that don't have a single track with
# momentum < 1.2 GeV.
trackerVeto= ldmxcfg.Producer("trackerVeto", "ldmx::TrackerVetoProcessor")

# Define the sequence of event processor to be run
p.sequence=[ecalSim2Rec, hcalDigis, ecalVeto, hcalVeto, simpleTrigger, trackerHitKiller, findableTrack, trackerVeto]

# Default to dropping all events
p.skimDefaultIsDrop()

# Use output of trigger module to decide what to keep
p.skimConsider("simpleTrigger")

# Determine the input file from the first argument to this script.
p.inputFiles = [sys.argv[1]]

# Provide the list of output files to produce, either one to contain the results of all input files or one output file name per input file name
p.outputFiles = ["recon.root"]

# Utility function to interpret and print out the configuration to the screen
p.printMe()
