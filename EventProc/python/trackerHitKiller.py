"""Configuration for TrackerhitKiller

Sets all parameters to reasonable defaults.

Examples
--------
>>> from LDMX.EventProc.trackerHitKiller import trackerHitKiller
>>> p.sequence.append( trackerHitKiller )
"""
#!/usr/bin/python

from LDMX.Framework import ldmxcfg

trackerHitKiller = ldmxcfg.Producer("trackerHitKiller", "ldmx::TrackerHitKiller")

# Configure
trackerHitKiller.parameters['hitEfficiency'] = 99.0
