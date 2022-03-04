from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

from LDMX.SimCore import simulator as sim
mySim = sim.simulator( "mySim" )
mySim.description = 'Hcal Muons and Neutrons'
mySim.setDetector( 'ldmx-det-v12' )
from LDMX.SimCore import generators as gen

# flat distribution of energy from 1GeV to 4GeV
# vertex on x-y plane close to front of side hcal/ecal
# angular distribution such that cos(theta) is flat from 0 to 1
ene_ang_pos_cmds = [
        '/gps/ene/type Lin',
        '/gps/ene/min 1 GeV',
        '/gps/ene/max 4 GeV',
        '/gps/ene/gradient 0.',
        '/gps/ene/intercept 1.',
        '/gps/ang/type cos',
        '/gps/pos/type Plane',
        '/gps/pos/shape Square',
        '/gps/pos/centre 0 0 220. mm', # start at side hcal
        '/gps/pos/halfx 500 mm',
        '/gps/pos/halfy 500 mm'
        ]

# one muon and one neutron both with the above initial kinematics
gps_cmds = ['/gps/particle mu-'] + ene_ang_pos_cmds + [
        '/gps/source/add 1',
        '/gps/particle neutron'
        ] + ene_ang_pos_cmds + [
        '/gps/source/multiplevertex True'
        ]

mySim.generators = [gen.gps('muon_neutron',gps_cmds)]

p.sequence = [ mySim ]

##################################################################
# Below should be the same for all sim scenarios

import os
import sys

p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.maxEvents = int(os.environ['LDMX_NUM_EVENTS'])

p.histogramFile = 'hist.root'
p.outputFiles = ['events.root']

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi

from LDMX.DQM import dqm

p.sequence.extend([
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        dqm.SimObjects(), dqm.HCalDQM()
        ])
