"""Configuration for TriggerProcessor

Sets all parameters to reasonable defaults.

Examples
--------
>>> from LDMX.EventProc.simpleTrigger import simpleTrigger
>>> p.sequence.append( simpleTrigger )
"""
#!/usr/bin/python

from LDMX.Framework import ldmxcfg

simpleTrigger = ldmxcfg.Producer("simpleTrigger", "ldmx::TriggerProcessor")

# set the mean noise in PE units

simpleTrigger.parameters["threshold"] = 1500.0
simpleTrigger.parameters["mode"] = 0
simpleTrigger.parameters["start_layer"] = 1
simpleTrigger.parameters["end_layer"] = 20
