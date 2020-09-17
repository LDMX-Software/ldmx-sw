"""SimpleCSVTableProvider

Specialization of ConditionsObjectProvider for simple tables indexed by detid
"""

from LDMX.Framework import ldmxcfg

class SimpleCSVTableEntry:
    def __init__(self, url):
        self.URL=url
        self.firstRun=-1
        self.lastRun=-1
        self.runType="any"

    def setIOV(self, firstRun, lastRun):
        self.firstRun=firstRun
        self.lastRun=lastRun
        
    def __str__(self) :
        """Stringify this ConditionsObjectProvider, creates a message with all the internal parameters.

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "\n  ConditionsObjectProvider(%s of class %s, tag='%s')"%(self.instanceName,self.className,self.tagName)
        if len(self.__dict__)>0:
            msg += "\n   Parameters:"
            for k, v in self.__dict__.items():
                msg += "\n    " + str(k) + " : " + str(v)

        return msg 

class SimpleCSVTableProvider(ldmxcfg.ConditionsObjectProvider):
    def __init__(self,objName, tagName, dataType, columns):
        super().__init__(objName,"ldmx::SimpleCSVTableProvider",tagName)
        self.dataType=dataType
        self.columns=columns
        self.entries=[]

    def validForever(self, url):
        self.entries.append(SimpleCSVTableEntry(url))

    def validForRuns(self, url, firstRun, lastRun):
        entry=SimpleCSVTableEntry(url)
        entry.setIOV(firstRun,lastRun)
        self.entries.append(entry)

    def validForAllRows(self, values):
        entry=SimpleCSVTableEntry("python:")
        entry.values=values
        self.entries.append(entry)
        
class SimpleCSVDoubleTableProvider(SimpleCSVTableProvider):
    def __init__(self,objName, tagName, columns):
        super().__init__(objName, tagName,"double",columns)
        
class SimpleCSVIntegerTableProvider(SimpleCSVTableProvider):
    def __init__(self,objName, tagName, columns):
        super().__init__(objName, tagName,"int",columns)

    


