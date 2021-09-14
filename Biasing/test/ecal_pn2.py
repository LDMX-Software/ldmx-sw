from LDMX.Framework import ldmxcfg
​
p=ldmxcfg.Process("ecal_pn2")
p.run = 1
​
from LDMX.SimCore import simulator
from LDMX.Ecal import EcalGeometry
from LDMX.Hcal import HcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions as ecal_conditions
import LDMX.Hcal.hcal_hardcoded_conditions as hcal_conditions
​
from LDMX.Biasing import ecal_2 
from LDMX.SimCore import generators
mysim = ecal_2.photo_nuclear('ldmx-det-v12', generators.single_4gev_e_upstream_tagger())
mysim.description = "ECal Non-Fiducial Test Simulation"
​
import LDMX.Ecal.digi as ecal_digi
import LDMX.Hcal.digi as hcal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.hcal as hcal_py
from LDMX.Recon.simpleTrigger import simpleTrigger
​
from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack
​
tsDigisUp   = TrigScintDigiProducer.up()
tsDigisTag  = TrigScintDigiProducer.tagger()
tsDigisDown = TrigScintDigiProducer.down()
​
#these three lines turn on trigger skimming
tsDigisUp   = TrigScintDigiProducer.up()
tsDigisTag  = TrigScintDigiProducer.tagger()
tsDigisDown = TrigScintDigiProducer.down()
​
simpleTrigger.end_layer = 20
p.skimDefaultIsDrop()
p.skimConsider(simpleTrigger.instanceName)
​
p.sequence=[ mysim,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(),
        ecal_vetos.EcalVetoProcessor(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        hcal_py.HcalVetoProcessor(),
        tsDigisUp, tsDigisTag, tsDigisDown,
        #trigScintTrack,
        simpleTrigger
        ]
​
p.outputFiles=["nonfiducial_test_production.root"]
​
p.maxTriesPerEvent = 1000 #not fully sure what this does
p.maxEvents = 1000
​
p.logFrequency = 100
​
print(p)
