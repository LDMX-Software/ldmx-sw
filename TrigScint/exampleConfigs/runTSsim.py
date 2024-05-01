import sys
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator


thisPassName="sim"
p = ldmxcfg.Process(thisPassName)

gunZpos=1100 #3000  #mm -- define as positive here, for file naming; set sign below
detV=2        #detector geometry version number 
beamXsmear=7.5 #mm
beamYsmear=20 #mm
noisePerEvent=1.  #average number of PEs from SiPM noise per event (gets scaled by nTimeSamples to be constant)
startSample=17.
if len(sys.argv) > 1 :
    nTimeSamples=int(sys.argv[1])
else :
    nTimeSamples=30 #config default is 5 
if len(sys.argv) > 2 :
    elecNoise=float(sys.argv[2])
else :
    elecNoise=1.5 #config default is 1.5
if len(sys.argv) > 3 :
    kExpo=float(sys.argv[3])
else :
    kExpo=0.1   #config default is 0.1
    
p.run = 10
p.maxEvents = 2000
p.outputFiles = ['testbeamSim_zNeg'+str(gunZpos)+'mm_beamSpot'+str(beamXsmear)+'x'+str(beamYsmear)+'mm_'+str(nTimeSamples)+'tSamp_eNoise'+str(elecNoise)+'_tauInv'+str(kExpo)+'_detV'+str(detV)+'_'+str(p.maxEvents)+'ev.root']
print("Producing output file: "+p.outputFiles[0])

gunZpos   =-float(gunZpos)  #get sign right, and make floats, to use as parameters
beamXsmear=float(beamXsmear)
beamYsmear=float(beamYsmear)

gun = generators.gun('particle_gun')
gun.particle = 'e-'
gun.direction = [0., 0., 1.]
gun.position = [0., 0., gunZpos]
gun.energy = 4.   #gev

simulation = simulator.simulator('test_TS')
simulation.generators=[gun]
simulation.setDetector('ldmx-hcal-prototype-v'+str(detV)+'.0')
simulation.beamSpotSmear = [beamXsmear, beamYsmear, 0] #mm, at start position

from LDMX.TrigScint.trigScint import TrigScintQIEDigiProducer
from LDMX.TrigScint.trigScint import TrigScintRecHitProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer

tsDigis = TrigScintQIEDigiProducer.up()
tsDigis.number_of_strips = 12
tsDigis.mean_noise=float(noisePerEvent/nTimeSamples)
tsDigis.maxts = nTimeSamples
tsDigis.elec_noise = elecNoise
tsDigis.expo_k = kExpo
tsDigis.sipm_gain = 2.e6
tsDigis.zeroSupp_in_pe=0.5
tsDigis.toff_overall = 25.*startSample
tsDigis.pe_per_mip = 110. #from fit to 2-hit clusters in data run 183, April4 2310, all plastic

tsRecHits = TrigScintRecHitProducer.up()
tsRecHits.pe_per_mip = tsDigis.pe_per_mip #pick it up here too
tsRecHits.gain = tsDigis.gain # same


tsCl = TrigScintClusterProducer.up()
tsCl.input_collection = tsRecHits.output_collection
tsCl.pad_time = 100.
tsCl.time_tolerance = 999.
#tsCl.verbosity = 3
tsCl.clustering_threshold = 30.  #to add in neighboring
tsCl.seed_threshold = 40.



from LDMX.TrigScint.trigScint import EventReadoutProducer

tsEv=EventReadoutProducer("eventLinearizer")
tsEv.input_pass_name=thisPassName
tsEv.input_collection=tsDigis.output_collection
tsEv.time_shift=0 #timeOffset

nChannels=12
gainList=[tsDigis.sipm_gain]*nChannels
pedList=[tsDigis.pedestal]*nChannels


from LDMX.TrigScint.trigScint import QIEAnalyzer

tsAna=QIEAnalyzer("plotMaker")
tsAna.inputPassName=thisPassName
tsAna.startSample=0
tsAna.pedestals=pedList
tsAna.gain=gainList

outname=p.outputFiles[0].replace(".root", "_plots.root")
p.histogramFile = outname 

p.sequence=[simulation,
                  tsDigis,
                  tsRecHits,
                  tsCl,
                  tsEv,
                  tsAna
                  ]


p.termLogLevel = 2 #0
