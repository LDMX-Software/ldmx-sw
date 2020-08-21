"""Configuration for TrackerhitKiller

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.EventProc.trackerHitKiller import trackerHitKiller
    p.sequence.append( trackerHitKiller )
"""

from LDMX.Framework import ldmxcfg

class TrackerHitKiller(ldmxcfg.Producer) :
    """Configuration for killing tracker hits"""

    def __init__(self,name,hitEfficiency) :
        super().__init__(name,'ldmx::TrackerHitKiller','EventProc')

        self.hitEfficiency = hitEfficiency

trackerHitKiller = TrackerHitKiller("trackerHitKiller", 99.0)

