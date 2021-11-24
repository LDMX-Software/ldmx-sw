"""UMN Specific DQM analyzers

for our test stand setup"""

from LDMX.Framework import ldmxcfg

class TestHgcRoc(ldmxcfg.Analyzer) :
    def __init__(self,input_name, input_pass = '',name = 'hgcroc') :
        super().__init__(name,'dqm::umn::TestHgcRoc','DQM')
        self.input_name = input_name
        self.input_pass = input_pass
