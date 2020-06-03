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
