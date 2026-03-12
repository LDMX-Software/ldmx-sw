from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('ecal_db')
from LDMX.Biasing import eat
from LDMX.Ecal import ecal_geometry
from LDMX.Hcal import hcal_geometry


p.sequence = [
    eat.dark_brem(
        100., #MeV - mass of A'
        'SimCore/G4DarkBreM/data/'
        'electron_tungsten_MaxE_4.0_MinE_0.2_RelEStep_0.1'
        '_UndecayedAP_mA_0.1_run_3000.csv.gz',
        'ldmx-det-v14' , #name of geometry to use
        )
    ]
p.max_tries_per_event = 1000
p.max_events = 100
p.output_files = [ 'ecal_db.root' ]
