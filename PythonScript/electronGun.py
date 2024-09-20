from LDMX.Framework import ldmxcfg
p=ldmxcfg.Process("electron")
p.libraries.append("libSimCore.so")
p.libraries.append("libHcal.so")
p.libraries.append("libEcal.so")

import argparse, sys
parser = argparse.ArgumentParser(f'ldmx fire {sys.argv[0]}')
parser.add_argument('energy',type=float)
parser.add_argument('nevents',default=100,type=int)
arg = parser.parse_args()

#nevents = 1000 # number of events
energy = arg.energy
nevents = arg.nevents

from LDMX.SimCore import simulator
from LDMX.SimCore import generators
sim = simulator.simulator("single_electron")
sim.setDetector( 'ldmx-det-v12' , True )
sim.description = "HCal electron"
sim.beamSpotSmear = [20., 80., 0.] #mm
#sim.beamSpotSmear = [100., 100., 0.] #mm
particle_gun = generators.gun( "single_electron_upstream_hcal")
particle_gun.particle = 'e-'
#position = 870.
position = 690.6 # back hcal
#position = 240.4 # ecal face
#position = 0.0
particle_gun.position = [ 0., 0., position ]  # mm
particle_gun.direction = [ 0., 0., 1.]
particle_gun.energy = energy
myGen = particle_gun
print(myGen)
sim.generators.append(myGen)

#p.outputFiles=['data/ngun_%.2fmm_%.2f_gev.root'%(position,energy)]
p.maxEvents = nevents
#p.outputFiles = ["eGun2_" + str(p.maxEvents) + "_smear_" + str(sim.beamSpotSmear[0]) 
                 #+ ", " + str(sim.beamSpotSmear[1]) + "_events_zpos_" + str(position) +".root"]

p.outputFiles = ["eGun2_" + str(p.maxEvents) + ".root"]
p.run = 11
p.sequence=[sim]

import LDMX.Ecal.digi as ecal_digi
import LDMX.Hcal.digi as hcal_digi

from LDMX.Ecal import EcalGeometry
geom = EcalGeometry.EcalGeometryProvider.getInstance()

from LDMX.Hcal import HcalGeometry
geom = HcalGeometry.HcalGeometryProvider.getInstance()

import LDMX.Hcal.hcal_hardcoded_conditions
digit = hcal_digi.HcalDigiProducer()
recon = hcal_digi.HcalRecProducer()

import LDMX.Ecal.ecal_hardcoded_conditions
p.sequence.extend([
    ecal_digi.EcalDigiProducer(),
    ecal_digi.EcalRecProducer(),
    digit,recon
])
