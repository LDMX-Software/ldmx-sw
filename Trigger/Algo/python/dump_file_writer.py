
from LDMX.Framework.ldmxcfg import Analyzer

class DumpFileWriter(Analyzer) :
    """Configuration for DumpFileWriter
    """

    def __init__(self, instance_name = 'myDumpFileWriter') :
        super().__init__(instance_name , 'trigger::DumpFileWriter','Trigger')
