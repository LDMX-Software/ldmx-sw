"""Configuration classes for sensitive detectors"""

from LDMX.SimCore import simcfg

class ScoringPlaneSD(simcfg.SensitiveDetector) :
    def __init__(self,subsystem) :
        super().__init__(f'{subsystem}_sp','simcore::ScoringPlaneSD','SimCore_SDs')

        self.collection_name = f'{subsystem.capitalize()}ScoringPlaneHits'
        self.match_substr = f'sp_{subsystem.lower()}' #depends on gdml

    def ecal() :
        return ScoringPlaneSD('ecal')

    def hcal() :
        return ScoringPlaneSD('hcal')

    def target() :
        return ScoringPlaneSD('target')

    def magnet() :
        return ScoringPlaneSD('magnet')

    def tracker() :
        sp = ScoringPlaneSD('tracker')
        sp.match_substr = 'sp_recoil'
        return sp

class TrackerSD(simcfg.SensitiveDetector) :
    def __init__(self,subsystem,subdet_id) :
        super().__init__(f'{subsystem}_TrackerSD','simcore::TrackerSD','SimCore_SDs')

        self.subsystem = subsystem
        self.subdet_id = subdet_id

        self.collection_name = f'{subsystem}SimHits'

    def tagger() :
        return TrackerSD('Tagger',1)

    def recoil() :
        return TrackerSD('Recoil',4)

class HcalSD(simcfg.SensitiveDetector) :
    def __init__(self) :
        super().__init__('hcal_sd', 'simcore::HcalSD','SimCore_SDs')

class EcalSD(simcfg.SensitiveDetector) :
    """
    enableHitContribs : bool, optional
        Should the simulation save contributions to Ecal sim hits?
    compressHitContribs : bool, optional
        Should the simulation compress contributions to Ecal sim hits by PDG ID?
    """
    def __init__(self) :
        super().__init__('ecal_sd', 'simcore::EcalSD','SimCore_SDs')
        self.enableHitContribs = True
        self.compressHitContribs = True

class TrigScintSD(simcfg.SensitiveDetector) :
    def __init__(self, which) :
        super().__init__(f'trig_scint_{which}_sd', 'simcore::TrigScintSD','SimCore_SDs')
        self.which = which

    def up() :
        return TrigScintSD('Up')

    def tag() :
        return TrigScintSD('Tagger')

    def down() :
        return TrigScintSD('Down')
