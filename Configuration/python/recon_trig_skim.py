#!/usr/bin/python

import sys
import os

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# Setup producers with default templates
from LDMX.EventProc.ecalDigis import ecalDigis
from LDMX.EventProc.ecalVeto import ecalVeto
from LDMX.EventProc.hcalDigis import hcalDigis
from LDMX.EventProc.hcalVeto import hcalVeto
from LDMX.EventProc.simpleTrigger import simpleTrigger
from LDMX.EventProc.trackerHitKiller import trackerHitKiller

p = ldmxcfg.Process("recon")
p.libraries.append("libEventProc.so")

findableTrack = ldmxcfg.Producer("findable", "ldmx::FindableTrackProcessor")

trackerVeto= ldmxcfg.Producer("trackerVeto", "ldmx::TrackerVetoProcessor")

p.sequence=[ecalDigis, hcalDigis, ecalVeto, hcalVeto, simpleTrigger, trackerHitKiller, findableTrack, trackerVeto]

# Default to dropping all events
p.skimDefaultIsDrop()

# Use output of trigger module to decide what to keep
p.skimConsider("simpleTrigger")

p.inputFiles = [sys.argv[1]]
p.outputFiles = ["recon.root"]

p.printMe()
