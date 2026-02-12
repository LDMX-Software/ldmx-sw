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
        self.url=url
        self.first_run=-1
        self.last_run=-1
        self.run_type="any"

    def setIOV(self, first_run, last_run):
        """Set the Interval Of Validity for this table entry

        Parameters
        ----------
        first_run : int
            First run number that this entry is valid for
        last_run : int
            Last run number that this entry is valid for
        """

        self.first_run=first_run
        self.last_run=last_run

    def __str__(self) :
        """Stringify this table entry

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "  TableEntry { "
        if ( self.first_run == -1 and self.last_run == -1 ) :
            msg += "Valid for All Time"
        else :
            msg += "Valid between %d and %d" %( self.first_run , self.last_run )

        msg += ", Run Type %s, URL %s }" %( self.run_type , self.url )

        return msg

class SimpleCSVTableProvider(ldmxcfg.ConditionsObjectProvider):
    """Provides a uniform table of a specific type

    Parametrs
    ---------
    obj_name : str
        Name of object that this provider provides (e.g. EcalGains)
    data_type : str
        Name of type of data stored in this table (e.g. "int" or "double")
    columns : list of str
        List of column names for this table
    conditions_baseURL : str
        Base location for URLs, filling the LDMX_CONDITION_BASEURL parameter inside any table's URL
    entriesURL : str
        URL to a CSV table mapping tables to specific intervals of validity.  Optional.
    """

    def __init__(self,obj_name,data_type, columns):
        super().__init__(obj_name,"conditions::SimpleCSVTableProvider",'Conditions')
        self.data_type=data_type
        self.columns=columns
        self.entries=[]
        self.conditions_base_url=''
        self.entries_url=''

    def validForever(self, url):
        """Add an entry to this provider that is valid forever and for all run types (data or MC)

        Parameters
        ----------
        url : str
            Location to download the table from
        """

        self.entries.append(SimpleCSVTableEntry(url))

    def validForRuns(self, url, first_run, last_run):
        """Add an entry to this provider that is valid between the input run numbers

        Parameters
        ----------
        url : str
            Location to download the table from
        first_run : int
            First run number that this entry is valid for
        last_run : int
            Last run number that this entry is valid for
        """

        entry=SimpleCSVTableEntry(url)
        entry.setIOV(first_run,last_run)
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
    def __init__(self,obj_name,columns):
        super().__init__(obj_name,"double",columns)

class SimpleCSVIntegerTableProvider(SimpleCSVTableProvider):
    """Provider for tables of integers"""
    def __init__(self,obj_name,columns):
        super().__init__(obj_name,"int",columns)

