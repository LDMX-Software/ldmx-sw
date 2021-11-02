import os
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("hitsmearing")

p.libraries.append("libTracking.so")

from LDMX.Tracking import tracking_hitsmearing


hitSmearing = tracking_hitsmearing.HitSmearingProcessor()

hitSmearing.input_hit_coll  = ["TaggerSimHits","RecoilSimHits"]
hitSmearing.output_hit_coll = ["SmearedTaggerSimHits","SmearedRecoilSimHits"]

hitSmearing.taggerSigma_u = 0.05
hitSmearing.taggerSigma_v = 0.25

hitSmearing.recoilSigma_u = 0.05
hitSmearing.recoilSigma_v = 0.25

p.sequence = [hitSmearing]

print(p.sequence)

p.inputFiles = [os.environ["LDMX_BASE"]+"/data_ldmx/mc_v12-4GeV-1e-inclusive_run1310001_t1601628859_reco.root"]
p.outputFiles = [os.environ["LDMX_BASE"]+'/data_ldmx/out.root']

p.termLogLevel=0

p.maxEvents = 1000
