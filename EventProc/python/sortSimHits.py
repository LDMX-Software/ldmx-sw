#!/usr/bin/python

import sys
import os
from LDMX.Framework import ldmxcfg

ecalSimHitSort = ldmxcfg.Producer("ecalSimHitSort", "ldmx::SimHitSortProcessor")
ecalSimHitSort.parameters["simHitCollection"]="EcalSimHits"
ecalSimHitSort.parameters["outputCollection"]="SortedEcalSimHits"
hcalSimHitSort = ldmxcfg.Producer("hcalSimHitSort", "ldmx::SimHitSortProcessor")
hcalSimHitSort.parameters["simHitCollection"]="HcalSimHits"
hcalSimHitSort.parameters["outputCollection"]="SortedHcalSimHits"

p = ldmxcfg.Process("sort")
p.libraries.append("ldmx-sw-install/lib/libEventProc.so")

p.sequence = [ecalSimHitSort,hcalSimHitSort]

p.inputFiles = ["/nfs/slac/g/ldmx/data/mc/v2/4pt0_gev_electrons_ecal_photonuc_v2_fieldmap/4pt0_gev_electrons_ecal_photonuc_v2_fieldmap_10to6_04752.root"]
p.outputFiles = ["recon_test.root"]
