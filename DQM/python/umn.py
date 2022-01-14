"""UMN Specific DQM analyzers

for our test stand setup"""

from LDMX.Framework import ldmxcfg

class TestHgcRoc(ldmxcfg.Analyzer) :
    def __init__(self,input_name, pedestal_table, input_pass = '',name = 'hgcroc') :
        super().__init__(name,'dqm::umn::TestHgcRoc','DQM')
        self.input_name = input_name
        self.input_pass = input_pass
        self.pedestal_table = pedestal_table

        from LDMX.Conditions.SimpleCSVTableProvider import SimpleCSVIntegerTableProvider
        t = SimpleCSVIntegerTableProvider(pedestal_table,["PEDESTAL"])
        t.validForever(f'file://{pedestal_table}')

