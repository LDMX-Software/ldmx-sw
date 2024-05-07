from LDMX.Framework import ldmxcfg
p=ldmxcfg.Process("v14_pn_deep_resim")

from LDMX.Ecal import EcalGeometry
from LDMX.Hcal import HcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions as ecal_conditions
import LDMX.Hcal.hcal_hardcoded_conditions as hcal_conditions
from LDMX.Biasing import ecal
from LDMX.SimCore import generators
from LDMX.Biasing import filters

det = 'ldmx-det-v14-8gev'
#mysim = ecal.deep_photo_nuclear(det, generators.single_8gev_e_upstream_tagger(), bias_threshold = 5010., processes=['conv','phot)'], ecal_min_Z = 300.)
mysim = ecal.deep_photo_nuclear(det, generators.single_8gev_e_upstream_tagger(), bias_threshold = 3000., processes=['conv','phot)'], ecal_min_Z = 400.)
mysim.description = "ECal Deep Donversion Test Re-Simulation"
#mysim.actions.extend([filters.TargetBremFilter()]),

from LDMX.Biasing import util
#mysim.actions.append( util.StepPrinter(1) )
#step = util.StepPrinter(track_id=1, depth=3)
#mysim.actions.extend([step])

import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Biasing.filters as filters

p.inputFiles = [f'events_pn_deep_test.root'] 
p.outputFiles = [f'events_pn_deep_test_resim.root']
p.histogramFile = f'hist_pn_deep_test_resim.root'

p.maxTriesPerEvent = 10000
p.maxEvents = 1000
p.run = 20
p.logFrequency = 100
p.termLogLevel = 0

p.sequence=[mysim.resimulate(which_events = [1]),
#        mysim.actions.extend([filters.TargetBremFilter()]),
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_vetos.EcalVetoProcessor()
        ]
      
from LDMX.DQM import dqm
p.sequence.append(dqm.SampleValidation())
p.sequence.extend(dqm.ecal_dqm)
