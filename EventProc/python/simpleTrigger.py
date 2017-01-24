#!/usr/bin/python

from LDMX.Framework import ldmxcfg

simpleTrigger = ldmxcfg.Producer("simpleTrigger","TriggerProcessor")

# set the mean noise in PE units

simpleTrigger.parameters["threshold"]=12.0
simpleTrigger.parameters["mode"]=0
simpleTrigger.parameters["start_layer"]=1
simpleTrigger.parameters["end_layer"]=16


