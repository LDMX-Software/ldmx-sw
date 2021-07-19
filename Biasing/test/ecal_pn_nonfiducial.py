from LDMX.Framework import ldmxcfg
p=ldmxcfg.Process("v14_nonfid")

from LDMX.Ecal import EcalGeometry
from LDMX.Hcal import HcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions as ecal_conditions
import LDMX.Hcal.hcal_hardcoded_conditions as hcal_conditions
from LDMX.Biasing import ecal
from LDMX.SimCore import generators

mysim = ecal.nonfiducial_photo_nuclear('ldmx-det-v14-8gev', generators.single_8gev_e_upstream_tagger())
mysim.description = "ECal Non-Fiducial Test Simulation"

#from LDMX.Biasing import util
#mysim.actions.append( util.StepPrinter(1) )

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos

p.outputFiles = [f'events_nonfiducial_test_production.root']
p.histogramFile = f'hist_nonfiducial_test_production.root'

p.maxTriesPerEvent = 10000
p.maxEvents = 100
p.run = 2
p.logFrequency = 100
#p.termLogLevel = 0

p.sequence=[ mysim,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_vetos.EcalVetoProcessor()
        ]
      
from LDMX.DQM import dqm
p.sequence.extend(dqm.ecal_dqm)
