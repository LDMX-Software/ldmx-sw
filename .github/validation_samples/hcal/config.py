from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('test')

from LDMX.SimCore import simulator as sim


my_sim = sim.simulator( "my_sim" )
my_sim.description = 'Hcal Muons and Neutrons'
my_sim.setDetector( 'ldmx-det-v15-8gev' )
from LDMX.SimCore import generators as gen


# flat distribution of energy from 1 GeV to 8 GeV
# vertex on x-y plane close to front of side hcal/ecal
# angular distribution such that cos(theta) is flat from 0 to 1
ene_ang_pos_cmds = [
        '/gps/ene/type Lin',
        '/gps/ene/min 1 GeV',
        '/gps/ene/max 8 GeV',
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

my_sim.generators = [gen.gps('muon_neutron',gps_cmds)]

p.sequence = [ my_sim ]

##################################################################
# Below should be the same for all sim scenarios

import os


p.run = int(os.environ['LDMX_RUN_NUMBER'])
p.max_events = int(os.environ['LDMX_NUM_EVENTS'])

p.histogram_file = 'hist.root'
p.output_files = ['events.root']
p.logger.term_level = 1
p.logger.custom("GEANT4", level = 3)

import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.digi as hcal_digi_and_reco
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry


hcal_digi = hcal_digi_and_reco.HcalDigiProducer()
hcal_reco = hcal_digi_and_reco.HcalRecProducer()
hcal_simplified_digi_and_reco = hcal_digi_and_reco.HcalSimpleDigiAndRecProducer()
hcal_simplified_digi_and_reco.output_coll_name = 'SimplifiedHcalRecHits'

# Load hcal veto
import LDMX.Hcal.hcal as hcal


hcal_veto = hcal.HcalVetoProcessor()

from LDMX.DQM import dqm


simplified_hcal_dqm_back = dqm.HCalDQM( 'HcalSimpleRecoDQM', section = 0 )
simplified_hcal_dqm_back.rec_coll_name = hcal_simplified_digi_and_reco.output_coll_name

simplified_hcal_dqm_top = dqm.HCalDQM( 'HcalSimpleRecoDQM', section = 1 )
simplified_hcal_dqm_top.rec_coll_name = hcal_simplified_digi_and_reco.output_coll_name

dqm.hcal_dqm.extend([simplified_hcal_dqm_back, simplified_hcal_dqm_top])

p.sequence.extend([
        hcal_digi,
        hcal_reco,
        hcal_veto,
        hcal_simplified_digi_and_reco,
        dqm.SimObjects(),
        *dqm.hcal_dqm,
        ])

