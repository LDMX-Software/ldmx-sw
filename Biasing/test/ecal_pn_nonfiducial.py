from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("v14_nonfid")

import LDMX.Ecal.ecal_hardcoded_conditions as ecal_conditions
import LDMX.Hcal.hcal_hardcoded_conditions as hcal_conditions
from LDMX.Biasing import ecal
from LDMX.Ecal import ecal_geometry
from LDMX.Hcal import hcal_geometry
from LDMX.SimCore import generators


mysim = ecal.nonfiducial_photo_nuclear(
    "ldmx-det-v14-8gev", generators.single_8gev_e_upstream_tagger()
)
mysim.description = "ECal Non-Fiducial Test Simulation"

# from LDMX.Biasing import util
# mysim.actions.append( util.StepPrinter(1) )

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos


ecal_veto = ecal_vetos.EcalVetoProcessor(recoil_from_tracking=False)


p.output_files = ["events_nonfiducial_test_production.root"]
p.histogram_file = "hist_nonfiducial_test_production.root"

p.max_tries_per_event = 10
p.max_events = 10
p.run = 2
p.logger.term_level = 0

p.sequence = [
    mysim,
    ecal_digi.EcalDigiProducer(),
    ecal_digi.EcalRecProducer(),
    ecal_veto,
]

# from LDMX.DQM import dqm
# p.sequence.extend(dqm.ecal_dqm)
