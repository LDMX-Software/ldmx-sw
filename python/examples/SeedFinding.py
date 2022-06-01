import os
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("tracking")

p.libraries.append("libTracking.so")

from LDMX.Tracking import tracking_seedfinder

seeder = tracking_seedfinder.SeedFinderProcessor()

p.sequence = [seeder]

print(p.sequence)

#You need to copy this file to the LDMX_BASE area for the example to work!
#/nfs/slac/g/ldmx/data/mc20/v12/4.0GeV/v2.2.1-batch8/mc_v12-4GeV-1e-ecal_photonuclear_run1075000_t1600764279.root

p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]
p.outputFiles = [os.environ["LDMX_BASE"]+'/data_ldmx/seeds.root']

p.termLogLevel=0

p.maxEvents = 10000
