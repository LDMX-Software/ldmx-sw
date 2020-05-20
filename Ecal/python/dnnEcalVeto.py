
from LDMX.Framework import ldmxcfg

from LDMX.Ecal.makePath import *

dnnEcalVeto = ldmxcfg.Producer("DNNEcalVeto", "ldmx::DNNEcalVetoProcessor")
dnnEcalVeto.parameters["debug"] = False
dnnEcalVeto.parameters["model_path"] = makeBDTPath("particle-net_ecal_v9")
dnnEcalVeto.parameters["disc_cut"] = -1.
dnnEcalVeto.parameters["collection_name"] = "EcalVetoDNN"
