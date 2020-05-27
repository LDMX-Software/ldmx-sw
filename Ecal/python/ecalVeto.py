"""Configuration for Ecal veto

Examples
--------
>>> from LDMX.Ecal.ecalVeto import ecalVeto
>>> p.sequence.append( ecalVeto )
"""

from LDMX.Framework import ldmxcfg

from LDMX.Ecal.makePath import *

ecalVeto = ldmxcfg.Producer("EcalVeto","ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 34
ecalVeto.parameters["do_bdt"] = True
ecalVeto.parameters["bdt_file"] = makeBDTPath( "gabrielle" )
ecalVeto.parameters["cellxy_file"] = makeCellXYPath()
ecalVeto.parameters["disc_cut"] = 0.99
ecalVeto.parameters["collection_name"] = "EcalVeto"
