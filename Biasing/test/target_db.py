from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('target_db')
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
from LDMX.Biasing import target


p.sequence = [
    target.dark_brem(
        100., #MeV - mass of A'
        'SimCore/G4DarkBreM/data/electron_tungsten_MaxE_4.0_MinE_0.2_RelEStep_0.1_UndecayedAP_mA_0.1_run_3000.csv.gz',
        'ldmx-det-v14', #name of geometry to use
        )
    ]
p.maxEvents = 1000
p.maxTriesPerEvent = 1000
p.outputFiles = [ 'target_db.root' ]
