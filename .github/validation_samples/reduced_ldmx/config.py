from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxTriesPerEvent = 100

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import simulator as sim

#myGun = gen.single_4gev_e_upstream_tagger()
myGun = gen.multi( "mgpGen" )
myGun.vertex = [ 0., 0., -880] # mm
myGun.momentum = [0.,0.,4000.] # MeV
myGun.nParticles = 1
myGun.pdgID = 11
myGun.enablePoisson = False #True   

mySim = sim.simulator( "mySim" ) # Build simulator object
det = 'ldmx-reduced-v1'
mySim.setDetector(det, True )
mySim.beamSpotSmear = [20.,80.,0.]
mySim.description = 'Reduced ECal Electron Gun Test Simulation'

mySim.generators = [ myGun ]
p.sequence = [ mySim ]
p.termLogLevel = 0

import os
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])
p.run = int(os.environ['LDMX_RUN_NUMBER'])

p.histogramFile = f'hist.root'
p.outputFiles = [f'events.root']

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi

ecalVeto = ecal_vetos.EcalVetoProcessor()
ecalVeto.num_ecal_layers = 6
ecalVeto.beam_energy = 4000.

from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack
ts_digis = [
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
        ]
for d in ts_digis :
    d.randomSeed = 1

from LDMX.Recon.electronCounter import ElectronCounter
from LDMX.Recon.simpleTrigger import TriggerProcessor

count = ElectronCounter(1,'ElectronCounter')
count.input_pass_name = ''

from LDMX.DQM import dqm

p.sequence.extend([
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecalVeto,
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        *ts_digis,
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
        trigScintTrack, 
        count, TriggerProcessor('trigger', 4000.),
       ] + dqm.ecal_dqm)
