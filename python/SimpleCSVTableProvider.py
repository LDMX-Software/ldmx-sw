"""SimpleCSVTableProvider

Specialization of ConditionsObjectProvider for simple tables indexed by detid
"""

from LDMX.Framework import ldmxcfg

class SimpleCSVTableItem:
    def __init__(self, name, url, dataType, columns):
        self.name=name
        self.URL=url
        self.dataType=dataType;
        self.columns=columns

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
    def __init__(self,instanceName, tagName):
        super().__init__(instanceName,"ldmx::SimpleCSVTableProvider",tagName)
        self.provides=[]

    def provideIntegerTable(self, name, url, columns):
        self.provides.append(SimpleCSVTableItem(name,url,"int",columns))

    def provideDoubleTable(self, name, url, columns):
        self.provides.append(SimpleCSVTableItem(name,url,"double",columns))

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
