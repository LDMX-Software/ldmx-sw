from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('test')

p.maxEvents = 10
p.run = 1
p.histogramFile = 'test/inclusive.root'
p.outputFiles = ['events/inclusive.root']

from LDMX.SimCore import simulator as sim
mySim = sim.simulator( "mySim" )
mySim.setDetector( 'ldmx-det-v12' )
from LDMX.SimCore import generators as gen
mySim.generators.append( gen.single_4gev_e_upstream_tagger() )
mySim.description = 'Basic test Simulation'

import LDMX.Ecal.EcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Hcal.HcalGeometry
import LDMX.Hcal.hcal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi
import LDMX.Ecal.vetos as ecal_vetos
import LDMX.Hcal.digi as hcal_digi

from LDMX.TrigScint.trigScint import TrigScintDigiProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer
from LDMX.TrigScint.trigScint import trigScintTrack
tsDigisUp   = TrigScintDigiProducer.up()
tsDigisTag  = TrigScintDigiProducer.tagger()
tsDigisDown = TrigScintDigiProducer.down()
tsDigisUp.randomSeed = 1
tsDigisTag.randomSeed = 1
tsDigisDown.randomSeed = 1
clTag=TrigScintClusterProducer.tagger()
clUp=TrigScintClusterProducer.up()
clDown=TrigScintClusterProducer.down()

from LDMX.DQM import dqm

p.sequence=[ mySim,
        ecal_digi.EcalDigiProducer(),
        ecal_digi.EcalRecProducer(), 
        ecal_vetos.EcalVetoProcessor(),
        hcal_digi.HcalDigiProducer(),
        hcal_digi.HcalRecProducer(),
        tsDigisUp, tsDigisTag, tsDigisDown,
        clTag, clUp, clDown,
        trigScintTrack
        ] + dqm.trigScint_dqm

