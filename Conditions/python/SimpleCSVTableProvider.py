"""SimpleCSVTableProvider

Specialization of ConditionsObjectProvider for simple tables indexed by detid
"""

from LDMX.Framework import (
    ConditionsObjectProvider,
    conditions_object_provider,
    parameter_set,
)


@parameter_set
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
    first_run: int
        first run number (inclusive) the entry is valid for
    last_run: int
        last run number (inclusive) the entry is valid for
    run_type: str
        data (real), mc (simulation), or any (both)
    values: list[float|int]
        values for each of the columns hardcoded instead of read from a file
    """

    url: str
    first_run: int = -1
    last_run: int = -1
    run_type: str = "any"
    values: list[float | int] = []


@conditions_object_provider(
    "UNDEFINED", "conditions::SimpleCSVTableProvider", "Conditions"
)
class SimpleCSVTableProvider(ConditionsObjectProvider):
    """Provides a uniform table of a specific type

    Parametrs
    ---------
    obj_name : str
        Name of object that this provider provides
        (e.g. EcalGains)
    data_type : str
        Name of type of data stored in this table
        (e.g. "int" or "double")
    columns : list of str
        List of column names for this table
    conditions_base_url : str
        Base location for URLs, filling the LDMX_CONDITION_BASEURL parameter
        inside any table's URL
    entries_url : str
        URL to a CSV table mapping tables to specific intervals of validity.  Optional.
    """

    obj_name: str
    data_type: str
    columns: list[str]
    conditions_base_url: str = ""
    entries_url: str = ""
    entries: list[SimpleCSVTableEntry] = []

    def __post_init__(self):
        self.object_name = self.obj_name

    def valid_forever(self, url):
        """Add an entry to this provider that is valid
        forever and for all run types (data or MC)

        Parameters
        ----------
        url : str
            Location to download the table from
        """

        self.entries.append(SimpleCSVTableEntry(url=url))

    def valid_for_runs(self, url, first_run, last_run):
        """Add an entry to this provider that is valid
        between the input run numbers

        Parameters
        ----------
        url : str
            Location to download the table from
        first_run : int
            First run number that this entry is valid for
        last_run : int
            Last run number that this entry is valid for
        """
        self.entries.append(
            SimpleCSVTableEntry(url=url, first_run=first_run, last_run=last_run)
        )

    def valid_for_all_rows(self, values):
        """Define a value to use for all rows instead of
        downloading the table from a URL

        Parameters
        ----------
        values : list
            A value for each column to use for all rows
        """

        self.entries.append(SimpleCSVTableEntry(url="python:", values=values))


def simple_csv_double_table_provider(obj_name, columns):
    """Factory for a SimpleCSVTableProvider for tables of doubles"""
    return SimpleCSVTableProvider(
        obj_name=obj_name, data_type="double", columns=columns
    )


def simple_csv_integer_table_provider(obj_name, columns):
    """Factory for a SimpleCSVTableProvider for tables of integers"""
    return SimpleCSVTableProvider(obj_name=obj_name, data_type="int", columns=columns)
