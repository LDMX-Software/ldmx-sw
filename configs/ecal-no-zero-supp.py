from LDMX.Framework import ldmxcfg

p=ldmxcfg.Process("nozero")
p.run = 1

from LDMX.SimCore import simulator
import LDMX.Ecal.EcalGeometry
sim = simulator.simulator("mySim")
sim.setDetector( 'ldmx-det-v12', True  )
sim.description = "ECal photo-nuclear, xsec bias 450"
sim.beamSpotSmear = [20., 80., 0]
from LDMX.SimCore import generators
sim.generators = [ generators.single_4gev_e_upstream_tagger() ]
sim.generators[0].particle = 'geantino'

import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi

digi_producer = ecal_digi.EcalDigiProducer()
digi_producer.zero_suppression = False

rec_producer = ecal_digi.EcalRecProducer()

p.sequence=[ sim, digi_producer, rec_producer ]

p.outputFiles=["geantino_ecal_no_zero_suppression.root"]
p.maxEvents = 10

