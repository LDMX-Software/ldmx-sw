"""SimpleCSVTableProvider

Specialization of ConditionsObjectProvider for simple tables indexed by detid
"""

from LDMX.Framework import ldmxcfg

class SimpleCSVTableEntry:
    """One entry in the providing table

    This defines an "entry" in the table provider corresponding
    to a certain Interval Of Validity (IOV). The entry can be
    be valid between two run number, over all time, only for MC,
    only for Data, or for both. The entry is retrieved from a
    URL location.

    Parameters
    ----------
    url : str
        URL to download the table entry from
    """

    def __init__(self, url):
        self.URL=url
        self.firstRun=-1
        self.lastRun=-1
        self.runType="any"

    def setIOV(self, firstRun, lastRun):
        """Set the Interval Of Validity for this table entry

        Parameters
        ----------
        firstRun : int
            First run number that this entry is valid for
        lastRun : int
            Last run number that this entry is valid for
        """

        self.firstRun=firstRun
        self.lastRun=lastRun
        
    def __str__(self) :
        """Stringify this table entry

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "  TableEntry { "
        if ( self.firstRun == -1 and self.lastRun == -1 ) :
            msg += "Valid for All Time"
        else :
            msg += "Valid between %d and %d" %( self.firstRun , self.lastRun )

        msg += ", Run Type %s, URL %s }" %( self.runType , self.URL )

        return msg 

class SimpleCSVTableProvider(ldmxcfg.ConditionsObjectProvider):
    """Provides a uniform table of a specific type

    Parametrs
    ---------
    objName : str
        Name of object that this provider provides (e.g. EcalGains)
    tagName : str
        Name of tag that this provider is generated from
    dataType : str
        Name of type of data stored in this table (e.g. "int" or "double")
    columns : list of str
        List of column names for this table
    """

    def __init__(self,objName, tagName, dataType, columns):
        super().__init__(objName,"ldmx::SimpleCSVTableProvider",tagName,'Conditions')
        self.dataType=dataType
        self.columns=columns
        self.entries=[]

    def validForever(self, url):
        """Add an entry to this provider that is valid forever and for all run types (data or MC)

        Parameters
        ----------
        url : str
            Location to download the table from
        """

        self.entries.append(SimpleCSVTableEntry(url))

    def validForRuns(self, url, firstRun, lastRun):
        """Add an entry to this provider that is valid between the input run numbers

        Parameters
        ----------
        url : str
            Location to download the table from
        firstRun : int
            First run number that this entry is valid for
        lastRun : int
            Last run number that this entry is valid for
        """

        entry=SimpleCSVTableEntry(url)
        entry.setIOV(firstRun,lastRun)
        self.entries.append(entry)

    def validForAllRows(self, values):
        """Define a value to use for all rows instead of downloading the table from a URL

        Parameters
        ----------
        values : list
            A value for each column to use for all rows
        """

        entry=SimpleCSVTableEntry("python:")
        entry.values=values
        self.entries.append(entry)
        
class SimpleCSVDoubleTableProvider(SimpleCSVTableProvider):
    """Provider for tables of doubles"""
    def __init__(self,objName, tagName, columns):
        super().__init__(objName, tagName,"double",columns)
        
class SimpleCSVIntegerTableProvider(SimpleCSVTableProvider):
    """Provider for tables of integers"""
    def __init__(self,objName, tagName, columns):
        super().__init__(objName, tagName,"int",columns)

