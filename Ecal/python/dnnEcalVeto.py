"""Configuration for DNN Ecal veto

Contains an object with helpful defaults set,
except the parameter 'disc_cut' which should be
set by the user if they want to actually have
the veto set storage hints.

Examples
--------
>>> from LDMX.Ecal.dnnEcalVeto import dnnEcalVeto
>>> p.sequence.append( dnnEcalVeto )
"""

from LDMX.Framework import ldmxcfg

from LDMX.Ecal.makePath import *

dnnEcalVeto = ldmxcfg.Producer("DNNEcalVeto", "ldmx::DNNEcalVetoProcessor")
dnnEcalVeto.parameters["debug"] = False
dnnEcalVeto.parameters["model_path"] = makeBDTPath("particle-net_ecal_v9")
dnnEcalVeto.parameters["disc_cut"] = -1.
dnnEcalVeto.parameters["collection_name"] = "EcalVetoDNN"
