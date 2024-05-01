import sys
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

thisPassName="sim"
p = ldmxcfg.Process(thisPassName)

from LDMX.Hcal import HcalGeometry
#from LDMX.Ecal import EcalGeometry

nEvents = 4000
killChan8 = True # toggle to kill all signal from channel 8 (dead in testbeam)
nElectrons=2
beamEnergy=4.  #in GeV

nChannels=12
gainList=[2e6]*nChannels
pedList=[6.]*nChannels

if killChan8 :
    gainList[8]=gainList[8]*gainList[8]  #to make the hit PEs very small
    killString="killChan8_"


gunZpos=800 #3000  #mm -- define as positive here, for file naming; set sign below
detV=2.0        #detector geometry version number 
beamXsmear=7.5 #mm    7.5mm for reasonable efficiency, larger than TS module to get more empty (no MIP)  events. set to 150mm for large noise 
beamYsmear=20  #mm    20 mm      -- " --, set to 200 for large noise 
noisePerEvent=0.08  #average number of PEs from SiPM noise per event (gets scaled by nTimeSamples to be constant) -- 0.1 per event is taken from run183 
startSample=15

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
    
p.run = 1
p.maxEvents = nEvents
p.outputFiles = ['testbeamSim_'+str(nElectrons)+'e_zNeg'+str(gunZpos)+'mm_beamSpot'+str(beamXsmear)+'x'+str(beamYsmear)+'mm_'+str(nTimeSamples)+'tSamp_eNoise'+str(elecNoise)+'_tauInv'+str(kExpo)+'_detV'+str(detV)+'_'+killString+str(p.maxEvents)+'ev.root']
print("Producing output file: "+p.outputFiles[0])

gunZpos   =-float(gunZpos)  #get sign right, and make floats, to use as parameters
beamXsmear=float(beamXsmear)
beamYsmear=float(beamYsmear)

#gunZpos=-1404. #mm
#gunZpos=-1504. #mm
#gunZpos=-1204. #mm

#------ set up beam simulation -------


mpgGen = generators.multi( "mgpGen" ) # this is the line that actually creates the generator
mpgGen.vertex = [ 0., 0., gunZpos ] # mm
mpgGen.nParticles = nElectrons
mpgGen.pdgID = 11
mpgGen.enablePoisson = False #True
mpgGen.momentum = [ 0., 0., beamEnergy ]

gun = generators.gun('particle_gun')
gun.particle = 'e-'
gun.direction = [0., 0., 1.]
gun.position = [0., 0., gunZpos]
gun.energy = beamEnergy   #gev

simulation = simulator.simulator('test_TS')
simulation.generators=[mpgGen] #gun]
simulation.setDetector('ldmx-hcal-prototype-v'+str(detV))
#simulation.setDetector('ldmx-hcal-prototype-v1.0')
simulation.beamSpotSmear = [beamXsmear, beamYsmear, 0] #mm, at start position
simulation.description = "Inclusive "+str(beamEnergy)+" GeV electron events, "+str(nElectrons)+"e"


#### the digi and tb hit step not fully implemented !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#------ set up digitization -------

from LDMX.TrigScint.trigScint import TrigScintQIEDigiProducer

tsDigis = TrigScintQIEDigiProducer.pad1() #up() #
tsDigis.input_collection= "TriggerPadUpSimHits"
tsDigis.number_of_strips = 12
tsDigis.mean_noise=float(noisePerEvent/nTimeSamples)

tsDigis.maxts = nTimeSamples
tsDigis.elec_noise = elecNoise
tsDigis.expo_k = kExpo
tsDigis.sipm_gain = 2.e6
tsDigis.zeroSupp_in_pe=0.5
tsDigis.toff_overall = 25.*startSample
tsDigis.pe_per_mip = 110. #from fit to 2-hit clusters in data run 183, April4 2310, all plastic


#------ set up event readout linearization -------

from LDMX.TrigScint.trigScint import EventReadoutProducer

tsEv=EventReadoutProducer("eventLinearizer")
tsEv.input_pass_name=thisPassName
tsEv.input_collection=tsDigis.output_collection #"trigScintQIEDigisPad1"
tsEv.time_shift=0 #timeOffset
tsEv.verbose = True 

#------ set up hit reconstruction -------

from LDMX.TrigScint.trigScint import TestBeamHitProducer

tbHits  =TestBeamHitProducer("tbHits")
tbHits.inputPassName=thisPassName
#if inputPassName=="sim" : #different conventions in sim and data
#    tbHits.inputCollection="trigScintQIEDigisPad1"
#else :
tbHits.inputCollection="QIEsamplesPad1"
tbHits.pedestals=pedList
tbHits.gain=gainList 
tbHits.startSample=startSample
tbHits.pulseWidth=12 #5 
tbHits.pulseWidthLYSO=12
tbHits.doCleanHits=True
tbHits.nInstrumentedChannels=nChannels


#------ set up clustering -------

from LDMX.TrigScint.trigScint import TestBeamClusterProducer

tbClusters  =TestBeamClusterProducer("tbClusters")
tbClusters.input_pass_name=thisPassName
tbClusters.input_collection=tbHits.outputCollection #"trigScintQIEDigisPad1" #
tbClusters.pad_time=100.
tbClusters.time_tolerance=999.
tbClusters.verbosity=0
tbClusters.clustering_threshold = 50.  #to add in neighboring

tbClusters3  =TestBeamClusterProducer("tbClusters3")
tbClusters3.input_pass_name=thisPassName
tbClusters3.input_collection=tbHits.outputCollection 
tbClusters3.output_collection=tbClusters.output_collection+"ThreeHits"
tbClusters3.max_cluster_width=3
tbClusters3.pad_time=100.
tbClusters3.time_tolerance=999.
tbClusters3.verbosity=0
tbClusters3.clustering_threshold = 50.  #to add in neighboring


#### ---- analyzers ------

from LDMX.TrigScint.trigScint import QIEAnalyzer

tsAna=QIEAnalyzer("plotMaker")
tsAna.inputPassName=thisPassName
tsAna.startSample=0
tsAna.pedestals=pedList
tsAna.gain=gainList


from LDMX.TrigScint.trigScint import TestBeamClusterAnalyzer

clAna2hit = TestBeamClusterAnalyzer("2-hitClusters")
clAna2hit.inputCollection=tbClusters.output_collection

clAna3hit = TestBeamClusterAnalyzer("3-hitClusters")
clAna3hit.inputCollection=tbClusters3.output_collection



outname=p.outputFiles[0].replace(".root", "_plots.root")
#p.outputFiles = [ outname ]
p.histogramFile = outname 

p.sequence=[simulation,
            tsDigis,
            tsEv,
            tbHits,
            tbClusters3,
            tbClusters,
            #clAna2hit,
            #clAna3hit
            #      tsEv,
            #      tsAna
                  ]


p.termLogLevel = 2
