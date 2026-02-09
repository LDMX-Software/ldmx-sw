from LDMX.Framework import ldmxcfg


p=ldmxcfg.Process("v14_conv_deep")

import LDMX.Ecal.ecal_hardcoded_conditions as ecal_conditions
import LDMX.Hcal.hcal_hardcoded_conditions as hcal_conditions
from LDMX.Biasing import ecal
from LDMX.Ecal import EcalGeometry
from LDMX.Hcal import HcalGeometry
from LDMX.SimCore import generators


det = 'ldmx-det-v14-8gev'
mysim = ecal.deep_photo_nuclear(det, generators.single_8gev_e_upstream_tagger(), bias_threshold = 5010., processes=['conv','phot)'], ecal_min_z = 400.)
#mysim = ecal.deep_photo_nuclear(det, generators.single_8gev_e_upstream_tagger(), bias_threshold = 5010., processes=['conv','phot)'], ecal_min_z = 200.)
mysim.description = "ECal Deep conversion Test Simulation"

#mysim.actions.append( util.StepPrinter(1) )
#step = util.StepPrinter(track_id=1, depth=3)
#mysim.actions.extend([step])
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
from LDMX.Biasing import util


p.output_files = ['events_pn_deep_test.root']
p.histogram_file = 'hist_pn_deep_test.root'

p.max_tries_per_event = 10000
p.max_events = 10
p.run = 20
p.log_frequency = 100
p.term_log_level = 0

p.sequence=[ mysim,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_vetos.EcalVetoProcessor()
        ]

from LDMX.DQM import dqm


p.sequence.append(dqm.SampleValidation())
p.sequence.extend(dqm.ecal_dqm)
