################################################################################
# @file triggerPad.py
# Define analyzers for DQM of trigger pad
################################################################################

from LDMX.Framework import ldmxcfg

################################################################################
# @func sim
# Analyzers focusing on simulated hits of trigger pads
################################################################################
def sim() :
    trigScintUp = ldmxcfg.Analyzer("TrigScintSimDQMUp", "ldmx::TrigScintDQM")
    trigScintUp.parameters["hit_collection"] = "TriggerPadUpSimHits"
    trigScintUp.parameters["pad"] = "up"
    
    trigScintTag = ldmxcfg.Analyzer("TrigScintSimDQMTag", "ldmx::TrigScintDQM")
    trigScintTag.parameters["hit_collection"] = "TriggerPadTaggerSimHits"
    trigScintTag.parameters["pad"] = "tag"
    
    trigScintDown = ldmxcfg.Analyzer("TrigScintSimDQMDown", "ldmx::TrigScintDQM")
    trigScintDown.parameters["hit_collection"] = "TriggerPadDownSimHits"
    trigScintDown.parameters["pad"] = "down"

    return [ trigScintUp , tricScintTag , trigScintDown ]

################################################################################
# @func digis
# Analyzers focusing on digitized hits of trigger pads
################################################################################
def digis() :
    trigScintUp = ldmxcfg.Analyzer("TrigScintHitDQMUp", "ldmx::TrigScintHitDQM")
    trigScintUp.parameters["hit_collection"] = "trigScintDigis"
    trigScintUp.parameters["pad"] = "up"
    
    trigScintTag = ldmxcfg.Analyzer("TrigScintHitDQMTag", "ldmx::TrigScintHitDQM")
    trigScintTag.parameters["hit_collection"] = "trigScintDigisTag"
    trigScintTag.parameters["pad"] = "tag"
    
    trigScintDown = ldmxcfg.Analyzer("TrigScintHitDQMDown", "ldmx::TrigScintHitDQM")
    trigScintDown.parameters["hit_collection"] = "trigScintDigisDn"
    trigScintDown.parameters["pad"] = "down"

    return [ trigScintUp , tricScintTag , trigScintDown ]
