"""Configuration for TrigScintDigiProducers

Sets all parameters to reasonable defaults.

Examples
--------
>>> from LDMX.EventProc.trigScintDigis import *
>>> p.sequence.extend([ trigScintDigis , trigScintDigisDn , trigScintDigisTag ])
"""
#!/usr/bin/python

from LDMX.Framework import ldmxcfg

trigScintDigis = ldmxcfg.Producer("trigScintDigis", "ldmx::TrigScintDigiProducer")

trigScintDigis.parameters["mean_noise"] = 0.02
trigScintDigis.parameters["number_of_strips"] = 50
trigScintDigis.parameters["number_of_arrays"] = 1
trigScintDigis.parameters["mev_per_mip"] = 0.4
trigScintDigis.parameters["pe_per_mip"] = 10.
trigScintDigis.parameters["input_collection"]="TriggerPadUpSimHits"
trigScintDigis.parameters["output_collection"]="trigScintDigisUp"

trigScintDigisDn = ldmxcfg.Producer("trigScintDigis", "ldmx::TrigScintDigiProducer")

trigScintDigisDn.parameters["mean_noise"] = 0.02
trigScintDigisDn.parameters["number_of_strips"] = 50
trigScintDigisDn.parameters["number_of_arrays"] = 1
trigScintDigisDn.parameters["mev_per_mip"] = 0.4
trigScintDigisDn.parameters["pe_per_mip"] = 10.
trigScintDigisDn.parameters["input_collection"]="TriggerPadDownSimHits"
trigScintDigisDn.parameters["output_collection"]="trigScintDigisDn"

trigScintDigisTag = ldmxcfg.Producer("trigScintDigis", "ldmx::TrigScintDigiProducer")

trigScintDigisTag.parameters["mean_noise"] = 0.02
trigScintDigisTag.parameters["number_of_strips"] = 50
trigScintDigisTag.parameters["number_of_arrays"] = 1
trigScintDigisTag.parameters["mev_per_mip"] = 0.4
trigScintDigisTag.parameters["pe_per_mip"] = 10.
trigScintDigisTag.parameters["input_collection"]="TriggerPadTaggerSimHits"
trigScintDigisTag.parameters["output_collection"]="trigScintDigisTag"
