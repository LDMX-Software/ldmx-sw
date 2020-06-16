"""Configured EcalDigiVerifier python object

Contains an instance of EcalDigiVerifier that
has already been configured.

Examples
--------
    from LDMX.DQM.ecalDigiVerify import exalDigiVerify
"""

from LDMX.Framework import ldmxcfg

ecalDigiVerify = ldmxcfg.Analyzer("EcalDigiVerify", "ldmx::EcalDigiVerifier")

ecalDigiVerify.parameters["ecalSimHitColl"] = "EcalSimHits"
ecalDigiVerify.parameters["ecalSimHitPass"] = "" #use whatever pass is available

ecalDigiVerify.parameters["ecalRecHitColl"] = "EcalRecHits"
ecalDigiVerify.parameters["ecalRecHitPass"] = "" #use whatever pass is available

##
# Number of SimHits per each cell
#
# Only including cells that have at least one hit.
# Integrates to number of rec hits.
ecalDigiVerify.build1DHistogram( "num_sim_hits_per_cell" ,
        "Number SimHits per ECal Cell (excluding empty rec cells)" , 20 , 0 , 20 )

##
# Total Rec Energy
#
# A perfect reconstruction would see a sharp gaussian around the total energy 
# being fired into the ECal in the sample used.
# (e.g. 4GeV electrons)
# Integrates to number of events.
# The maximum is 8000MeV.
ecalDigiVerify.build1DHistogram( "total_rec_energy"      ,
        "Total Reconstructed Energy in ECal [MeV]" , 800 , 0. , 8000. )

##
# SimHit E Dep vs Rec Hit Amplitude
#
# A perfect reconstruction would see a one-to-one linear relationship between these two.
# Integrates to number of Rec Hits.
# Aggregates EDeps from any SimHits in the same cell.
# The maximum is 25MeV.
ecalDigiVerify.build2DHistogram( "sim_edep__rec_amplitude" ,
        "Simulated [MeV]" , 100 , 0. , 50. ,
        "Reconstructed [MeV]" , 100 , 0. , 50. )

