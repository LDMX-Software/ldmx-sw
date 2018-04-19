#!/usr/bin/python

from LDMX.Framework import ldmxcfg

trackerHitKiller = ldmxcfg.Producer("trackerHitKiller", "ldmx::TrackerHitKiller")

# Configure
trackerHitKiller.parameters['hitEfficiency'] = 99.0
