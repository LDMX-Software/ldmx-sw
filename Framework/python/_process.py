from ._conditions_object_provider import ConditionsObjectProvider
from ._logger import Logger
from ._parameter_set import field, parameter_set
from ._processor import Processor
from ._rnss import RandomNumberSeedService


@parameter_set
class Process:
    """Process configuration object

    The python object that stores the necessary parameters for configuring
    a Process for ldmx-app to execute.

    Upon construction, the class-wide reference last_process is set
    and the rest of the attributes are set to sensible defaults.

    Parameters
    ----------
    pass_name : str
        Short reference name for this run of the process

    Attributes
    ----------
    last_process : Process
        Class-wide reference to the last Process object to be constructed
    max_events : int
        Maximum number events to process.
        If total_events is set, this will be ignored.
    min_events : int
        Index of the first events to process.
        The skipping process is relatively slow, if used for anything outside of debugging
        make a  skim to a new file and then run again rather than use this.
        Note: this skips events of *each* input file, you a single file only.
    max_tries_per_event : int
        Maximum number of attempts to make in a row before giving up on an event
        Only used in Production Mode (no input files)
        If total_events is set, this will be ignored.
    total_events : int
        Number of events we'd like to produce independetly of the number of tries it would take.
        Both max_events and max_tries_per_event will be ignored. Be warned about infinite loops!
    run : int
        Run number for this process
    input_files : list of strings
        Input files to read in event data from and process
    output_files : list of strings
        Output files to write out event data to after processing
    sequence : list of Producers and Analyzers
        list of event processors to pass the event bus objects to
    keep : list of strings
        list of rules to keep or drop objects from the event bus
    libraries : list of strings
        list of libraries to load before attempting to build any processors
    skim_default_is_keep : bool
        Flag to say whether to process should by default keep the event or not
    skim_rules : list of strings
        list of skimming rules for which processors the process should listen to when deciding whether to keep an event
    log_frequency : int
        Print the event number whenever its modulus with this frequency is zero
    logger : Logger
        configuration for logging system in ldmx-sw
    conditions_global_tag : str
        Global tag for the current generation of conditions
    conditions_object_providers : list of ConditionsObjectProviders
        list of the sources of calibration and conditions information
    random_number_seed_service : RandomNumberSeedService
        conditions object that provides random number seeds in a deterministic way

    See Also
    --------
    Producer : one type of event processor
    Analyzer : the other type of event processor
    """

    last_process = None

    pass_name: str
    max_events: int = -1
    min_events: int = -1
    max_tries_per_event: int = 1
    run: int = -1
    input_files: list[str] = []
    output_files: list[str] = []
    sequence: list[Processor] = []
    keep: list[str] = []
    libraries: list[str] = []
    skim_default_is_keep: bool = True
    skim_rules: list[str] = []
    log_frequency: int = -1
    logger: Logger = field(default_factory=Logger)
    compressionSetting: int = 9
    histogramFile: str = ""
    conditions_global_tag: str = "Default"
    conditions_object_providers: list[ConditionsObjectProvider] = []
    tree_name: str = "LDMX_Events"
    random_number_seed_service: RandomNumberSeedService = field(
        init=False, default_factory=RandomNumberSeedService
    )
    __legacy__ = {
        "termLogLevel": "logger.termLevel",
        "fileLogLevel": "logger.fileLevel",
        "logFileName": "logger.filePath",
    }

    def __post_init__(self):
        if Process.last_process is not None:
            raise Exception(
                "Process object is already created! You can only create one Process object in a script."
            )

        # needs last_process defined to update registries
        Process.last_process = self

        from . import _register

        self.libraries.extend(_register.library.__registry__)

        for cop in _register.conditions_object_provider.__registry__:
            self._declare_conditions_object_provider(cop)

        self.random_number_seed_service = RandomNumberSeedService()

    def _declare_conditions_object_provider(self, cop):
        """Declare a conditions object provider to be loaded with the process

        Parameters
        ----------
        cop
            provider to load with the process

        Warnings
        --------
        - Overrides an already declared COP with the passed COP if they are equal
        """

        cop.tag_name = Process.last_process.conditions_global_tag

        # check if the input COP matches one already declared
        #   if it does match, override the already declared one with the passed one
        for index, already_defined_cop in enumerate(self.conditions_object_providers):
            if cop == already_defined_cop:
                self.conditions_object_providers[index] = cop
                return

        self.conditions_object_providers.append(cop)

    def setConditionsGlobalTag(self, tag):
        """Set the global tag for all the ConditionsObjectProviders

        Parameters
        ----------
        tag : str
            Global generation tag to pass to all COPs
        """

        self.conditions_global_tag = tag
        for cop in self.conditions_object_providers:
            cop.tag_name = tag

    def skimDefaultIsSave(self):
        """Configure the process to by default keep every event."""

        self.skim_default_is_keep = True

    def skimDefaultIsDrop(self):
        """Configure the process to by default drop (not save) every event."""

        self.skim_default_is_keep = False

    def skimConsider(self, namePat):
        """Configure the process to listen to processors matching input.

        The list of skim rules has a rather complicated form, so it
        is better to use this helper function.

        Parameters
        ----------
        namePat : str
            Pattern for the processor instance_names to match for the Process to listen

        Example
        -------
            ecalVeto = ldmxcfg.Producer( 'ecalVeto' , 'EcalVetoProcessor' )
            # Setup of other parameters for the veto
            p.skimConsider( 'ecalVeto' )

        See Also
        --------
        skimConsiderLabelled

        """

        self.skim_rules.append(namePat)
        self.skim_rules.append("")

    def skimConsiderLabelled(self, namePat, labelPat):
        """Configure the process to listen to processors matching input.

        The list of skim rules has a rather complicated form, so it
        is better to use this helper function.

        Some uses of ``setStorageHint`` in processors include a "reason"
        for the given storage hint. The input label pattern is check
        on matching this "reason" for the storage hint.

        Parameters
        ----------
        namePat : str
            Pattern for the processor instance_names to match for the Process to listen
        labelPat : str
            Pattern for the storage hint reason to match for the Process to listen

        See Also
        --------
        skimConsider

        """
        self.skim_rules.append(namePat)
        self.skim_rules.append(labelPat)

    def setCompression(self, algorithm, level=9):
        """set the compression settings for any output files in this process

        We combine the compression settings here in the same way that ROOT
        does. This allows the compression settings to be passed along as
        one integer rather than two without any loss of generality.

        Look at ROOT's documentation for TFile to learn more
        about the different compression algorithms and levels available
        (as well as what integers to use). There is a summary table
        below.

        Algorithm           | int | root version
        ------------------- | --- | ------------
        ROOT global default |  0  | root-6.16
        ZLIB                |  1  | root-6.16
        LZMA                |  2  | root-6.16
        Old (ROOT 5)        |  3  | root-6.16
        LZ4                 |  4  | root-6.16
        ZSTD                |  5  | root-6.20

        Level 0 : no compression is applied
        Level 9 : maximum amount of compression available from algorithm

        Parameters
        ----------
        algorithm : int
            flag for the algorithm to use
        level : int
            flag for the level of compression to use
        """

        self.compressionSetting = algorithm * 100 + level

    def inputDir(self, indir):
        """Scan the input directory and make a list of input root files to read from it

        lists all files ending in '.root' in the input directory (not recursive).
        Extends the input_files list by these files.

        Parameters
        ----------
        indir : str
            Path to directory of event files to read in
        """

        import os

        fullPathDir = os.path.realpath(indir)
        self.input_files.extend(
            [
                os.path.join(fullPathDir, f)
                for f in os.listdir(fullPathDir)
                if os.path.isfile(os.path.join(fullPathDir, f)) and f.endswith(".root")
            ]
        )

    def parameterDump(self):
        """Recursively extract all configuration parameters for this process

        Only includes objects somehow attached to the process.
        """

        keys_to_skip = ["histograms", "libraries"]

        def extract(obj):
            """Extract the parameter from the input object"""

            if isinstance(obj, list):
                return [extract(o) for o in obj]
            elif hasattr(obj, "__dict__"):
                params = dict()
                for k in obj.__dict__:
                    if k not in keys_to_skip:
                        params[k] = extract(obj.__dict__[k])
                return params
            else:
                return obj

        return extract(self)


    def pause(self):
        """Print this Process and wait for user confirmation to continue

        Prints the process through the print function, and then
        waits for the user to press Enter to continue.
        """

        print(str(self))
        input("Press Enter to continue...")


    def __str__(self):
        """Stringify this object into a human readable, helpful form.

        This function creates a very large, multi-line string that reports (almost) all of the important
        details of this configured process.

        Returns
        -------
        str
            A human-readable, multi-line description of this process object
        """

        msg = "Process with pass name '%s'" % (self.pass_name)
        if self.run > 0:
            msg += "\n using run number %d" % (self.run)
        if self.max_events > 0:
            msg += "\n Maximum events to process: %d" % (self.max_events)
        else:
            msg += "\n No limit on maximum events to process"
        if len(self.conditions_object_providers) > 0:
            msg += "\n conditions_object_providers:\n - "
            msg += '\n  - '.join(str(cop) for cop in self.conditions_object_providers)
        msg += "\n Processor sequence:\n - "
        msg += '\n  - '.join(str(proc) for proc in self.sequence)
        if len(self.input_files) > 0:
            if len(self.output_files) == len(self.input_files):
                msg += "\n Files:"
                for i in range(0, len(self.input_files)):
                    msg += "\n  '%s' -> '%s'" % (
                        self.input_files[i],
                        self.output_files[i],
                    )
            else:
                msg += "\n Input files:"
                for afile in self.input_files:
                    msg += "\n  " + afile
                if len(self.output_files) > 0:
                    msg += "\n Output file: " + self.output_files[0]
        elif len(self.output_files) > 0:
            msg += "\n Output file: " + self.output_files[0]
        msg += "\n Skim rules:"
        if self.skim_default_is_keep:
            msg += "\n  Default: keep the event"
        else:
            msg += "\n  Default: drop the event"
        for i in range(0, len(self.skim_rules) - 1, 2):
            if self.skim_rules[i + 1] == "":
                msg += (
                    "\n  listen to hints from processors with names matching '%s'"
                    % (self.skim_rules[i])
                )
            else:
                msg += (
                    "\n  listen to hints with labels matching '%s' from processors with names matching '%s'"
                    % (self.skim_rules[i + 1], self.skim_rules[i])
                )
        if len(self.keep) > 0:
            msg += "\n Rules for keeping previous products:"
            for arule in self.keep:
                msg += "\n  " + arule
        if len(self.libraries) > 0:
            msg += "\n Shared libraries to load:"
            for afile in set(self.libraries):
                msg += "\n  " + afile

        return msg
