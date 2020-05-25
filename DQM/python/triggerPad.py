"""Configured TricScintDQM python objects"""

from LDMX.Framework import ldmxcfg

def sim() :
    """Analyzers focusing on simulated hits of trigger pads

    Defines three instances of TrigScintDQM Analyzers corresponding
    to the three trigger pads.

    Returns
    -------
    list of ldmxcfg.Analyzer s
        Configured to be TrigScintDQMs for the three trigger pads

    Examples
    --------
    Import and make histograms for the simulated trigger pad hits:
    >>> from LDMX.DQM import triggerPad
    >>> p.sequenc.extend(triggerPad.sim())
    """

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

def digis() :
    """Analyzers focusing on digitized hits of trigger pads

    Defines three instances of TrigScintDQM Analyzers corresponding
    to the three trigger pads.

    Returns
    -------
    list of ldmxcfg.Analyzer s
        Configured to be TrigScintDQMs for the three trigger pads

    Examples
    --------
    Import and make histograms for the digitized trigger pad hits:
    >>> from LDMX.DQM import triggerPad
    >>> p.sequenc.extend(triggerPad.digis())
    """

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
