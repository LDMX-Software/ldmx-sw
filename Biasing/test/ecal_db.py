from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_db')
from LDMX.Biasing import eat
from LDMX.Ecal import EcalGeometry
from LDMX.Hcal import HcalGeometry
p.sequence = [
    eat.dark_brem( 
        100., #MeV - mass of A'
        'SimCore/G4DarkBreM/data/electron_tungsten_MaxE_4.0_MinE_0.2_RelEStep_0.1_UndecayedAP_mA_0.1_run_3000.csv.gz',
        'ldmx-det-v14' , #name of geometry to use
        )
    ]
p.maxTriesPerEvent = 1000
p.maxEvents = 100
p.outputFiles = [ 'ecal_db.root' ]
