from LDMX.Framework.ldmxcfg import Analyzer

class HitSmearingAnalysis(Analyzer):

    def __init__(self,name="HitSmearingAnalysis") :
        super().__init__(name,'tracking::analysis::HitSmearingAnalysis','Tracking')
