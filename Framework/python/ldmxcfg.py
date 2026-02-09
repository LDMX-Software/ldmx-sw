"""ldmxcfg

Basic python configuration for ldmx-sw application
"""

from pathlib import Path


class EventProcessor:
    """An EventProcessor object

    This object contains the parameters that are necessary for a framework::EventProcessor to be configured.

    You should NOT use this class directly. Use one of the derived classes Producer or Analyzer for clarity.

    Parameters
    ----------
    instance_name : str
        Name of this copy of the producer object
    class_name : str
        Name (including namespace) of the C++ class that this processor should be
    module_name : str
        Name of module the C++ class is in (e.g. Ecal or SimCore)
        or full path to the library that should be loaded

    Attributes
    ----------
    histograms : list of histogram1D objects
        List of histogram configure objects for the HistogramPool to make for this processor

    See Also
    --------
    LDMX.Framework.ldmxcfg.Producer : Producer configuration object
    LDMX.Framework.ldmxcfg.Analyzer : Analyzer configuration object
    LDMX.Framework.histogram.histogram : histogram configuration object
    """

    def __init__(self, instance_name, class_name, module_name):
        self.instance_name=instance_name
        self.class_name=class_name
        self.histograms=[]

        if module_name.endswith('.so'):
            # assume user passed full path to library
            Process.add_library(module_name)
        else:
            # assume user passed name of module processor is compiled into
            Process.add_module(module_name)


    @classmethod
    def from_file(cls, source_file, class_name = None, needs = [], instance_name = None, compile_notice = True, **config_kwargs):
        """Construct an event processor "in place" from the passed source file

        Since Framework dynamically loads libraries containing processors after
        the python script has been fully run, we can compile a single-file processor
        into its own library that can then be loaded and run. This function puts
        the library next to the source file and only re-compiles if the source file's
        last modified time is newer than the library (or the library does not exist).

        Note
        ----
        Developing processors in this way is incredible inefficient, especially since it
        does not allow for code to be well organized and split across many files nor does it
        allow for two processors to share common code.
        If you find yourself defining more than one `class` within your new C++ processor,
        it is highly recommended to transition your workflow to including your processor as a
        part of ldmx-sw so that it can fully benefit from a build system.

        Parameters
        ----------
        source_file: str | Path
            path to source file to build into a processor (can be relative to where config is being run)
        class_name: str, default is name of source file
            name of C++ class that is the processor
            defaults to the name of the source file without an extension
        needs: list[str]
            Names of libraries that should be linked to the compiled processor in addition to 'Framework'
            which is linked be default.
            For example, one can gain access to the detector ID infrastructure with 'DetDescr' or the
            Ecal Event classes with 'Ecal_Event' (or 'Ecal/Event', or 'Ecal::Event')
        instance_name: str, default is class_name
            name to give to instance of this C++ processor
        compile_notice: bool, default is True
            print a notice when compilation is triggered
        config_kwargs: dict[str, Any]
            configuration parameters to give to the processor

        Examples
        --------
        A basic walkthrough is available online. https://ldmx-software.github.io/analysis/ldmx-sw.html
        
        If `MyAnalyzer.cxx` contains the class `MyAnalyzer`, then we can put

            p.sequence = [ ldmxcfg.Analyzer.from_file('MyAnalyzer.cxx') ]

        In our config script to run the analyzer on its own in the sequence.
        This default configuration only links the Framework library and so the analyzer
        would only be able to access the Framework and event objects.
        If you needed another library (for example, the 'DetDescr' library has the ID classes),
        one can also

            p.sequence = [ ldmxcfg.Analyzer.from_file('MyAnalyzer.cxx', needs = ['DetDescr']) ]

        To inform the compiler that it should link your analyzer with the 'DetDescr' library.
        **No removal of the library is done** so if you change the `needs` or some other parameter
        to `from_file` you should also remove the library file (`*.so`) before attempting to re-run.

        Returns
        -------
        EventProcessor
            built from the C++ source file and configured with the passed arguments
        """

        if not isinstance(source_file, Path):
            source_file = Path(source_file)
        if not source_file.is_file():
            raise ValueError(f'{source_file} is not accessible.')

        src = source_file.resolve()

        if class_name is None:
            # assume class name is name of file (no extension) if not provided
            class_name = src.stem

        if instance_name is None:
            # use class name for instance name if not provided
            instance_name = class_name

        lib = src.parent / f'lib{src.stem}.so'
        if not lib.is_file() or src.stat().st_mtime > lib.stat().st_mtime:
            if compile_notice:
                print(
                    f'Processor source file {src} is newer than its compiled library {lib}'
                    ' (or library does not exist), recompiling...'
                )
            import subprocess
            libs_to_link = set(['Framework']+needs)
            cmd = [
                'g++', '-std=c++20', '-fPIC', '-shared', # construct a shared library for dynamic loading
                '-o', str(lib), str(src), # define output file and input source file
            ]+[
                f'-l{lib}' for lib in libs_to_link
            ]+[
                '-I/usr/local/include/root', # include ROOT's non-system headers
                '-I@CMAKE_INSTALL_PREFIX@/include', # include ldmx-sw headers (if non-system)
                '-L@CMAKE_INSTALL_PREFIX@/lib', # include ldmx-sw libs (if non-system)
            ]
            if compile_notice:
                print(*cmd)
            subprocess.run(cmd, check=True)
            if compile_notice:
                print(f'done compiling {src}')

        instance = cls(instance_name, class_name, str(lib))
        for cfg_name, cfg_val in config_kwargs.items():
            setattr(instance, cfg_name, cfg_val)

        # load any dependency libraries at runtime as well
        for mod in needs:
            Process.add_module(mod)

        return instance


    def build_1d_histogram(self, name, xlabel, bins, xmin = None, xmax = None, weighted=False):
        """Make a 1D histogram 

        If xmin and xmax are not provided, bins is assumed to be
        the bin edges on the x-axis. If they are both provided,
        bins is assumed to be the number of bins on the x-axis.

        Parameters
        ----------
        name : str
            variable name of histogram
        xlabel : str
            title of x-axis of histogram
        bins : int OR list of floats OR list of str
            Number of bins on x-axis OR bin edges on x-axis OR string categories
        xmin : float
            Minimum edge of bins on x-axis
        xmax : float
            Maximum edge of bins on x-axis
        weighted: bool
            whether to keep track of sum of squared weights

        See Also
        --------
        LDMX.Framework.histogram.histogram : histogram configuration object
        """

        import LDMX.Framework.histogram as h
        the_bins = bins
        if xmin is not None and xmax is not None :
            the_bins = h.uniform_binning(bins,xmin,xmax)

        self.histograms.append(h.histogram(name, xlabel,the_bins, weighted=weighted))


    def build_2d_histogram(self, name,
            xlabel = '', xbins = 1, xmin = None, xmax = None,
            ylabel = '', ybins = 1, ymin = None, ymax = None,
            weighted = False) :
        """Create a 2D histogram

        If {x,y}min or {x,y}max are not provided, {x,y}bins is assumed
        to be the bin edges on the {x,y}-axis (or named string categories).
        If they are both provided,
        {x,y}-bins is assumed to be the number of bins on the {x,y}-axis.

        Parameters
        ----------
        name : str
            variable name of histogram
        xlabel : str
            title of x-axis of histogram
        xbins : int OR list of floats OR list of str
            Number of bins on x-axis OR list of bin edges on x-axis OR string categories
        xmin : float
            Minimum edge of bins on x-axis
        xmax : float
            Maximum edge of bins on x-axis
        ylabel : str
            title of y-axis of histogram
        ybins : int OR list of floats OR list of str
            Number of bins on y-axis OR list of bin edges on y-axis OR string categories
        ymin : float
            Minimum edge of bins on y-axis
        ymax : float
            Maximum edge of bins on y-axis
        weighted: bool
            whether to keep track of sum of squared weights

        See Also
        --------
        LDMX.Framework.histogram.histogram : histogram configuration object

        Examples
        --------

        When doing all uniform binning, you can specify the arguments by position.
            myProcessor.build_2d_histogram( 'dummy' ,
                'My X Axis' , 20 , 0. , 1. ,
                'My Y Axis' , 60 , 0. , 10. )

        When using variable binning, you have to use the parameter names.
            myProcessor.build_2d_histogram( 'dummy2' ,
                xlabel='My X Axis', xbins=[0.,1.,2.],
                ylabel='My Y Axis', ybins=60, ymin=0., ymax=10. )
        """

        import LDMX.Framework.histogram as h
        the_x_bins = xbins
        if xmin is not None and xmax is not None :
            the_x_bins = h.uniform_binning(xbins,xmin,xmax)

        the_y_bins = ybins
        if ymin is not None and ymax is not None :
            the_y_bins = h.uniform_binning(ybins,ymin,ymax)

        self.histograms.append(
                h.histogram(name, xlabel,the_x_bins, ylabel,the_y_bins, weighted=weighted)
                )


class Producer(EventProcessor):
    """A producer object.

    This object contains the parameters that are necessary for a framework::Producer to be configured.

    See Also
    --------
    LDMX.Framwork.ldmxcfg.EventProcessor : base class
    """

    def __init__(self, instance_name, class_name, module_name):
        super().__init__(instance_name,class_name, module_name)

    def __str__(self) :
        """Stringify this Producer, creates a message with all the internal parameters.

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "\n  Producer(%s of class %s)"%(self.instance_name,self.class_name)
        if len(self.__dict__)>0:
            msg += "\n   Parameters:"
            for k, v in self.__dict__.items():
                msg += "\n    " + str(k) + " : " + str(v)

        return msg

class Analyzer(EventProcessor):
    """A analyzer object.

    This object contains the parameters that are necessary for a framework::Analyzer to be configured.

    See Also
    --------
    LDMX.Framework.ldmxcfg.EventProcessor : base class
    """

    def __init__(self, instance_name, class_name, module_name):
        super().__init__(instance_name,class_name, module_name)

    def __str__(self) :
        """Stringify this Analyzer, creates a message with all the internal parameters.

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "\n  Analyzer(%s of class %s)"%(self.instance_name,self.class_name)
        if len(self.__dict__)>0:
            msg += "\n   Parameters:"
            for k, v in self.__dict__.items():
                msg += "\n    " + str(k) + " : " + str(v)

        return msg

class ConditionsObjectProvider:
    """A ConditionsObjectProvider

    This object contains the parameters that are necessary for a framework::ConditionsObjectProvider to be configured.

    In this constructor we also do two helpful processes.
    1. We append the module that this provider is in to the list of libraries to load
    2. We declare this provider so that the Process "knows" it exists and will load it into the run

    Parameters
    ----------
    object_name : str
        Name of the object this provider provides
    class_name : str
        Name (including namespace) of the C++ class of the provider
    module_name : str
        Name of module that this COP is compiled into (e.g. Ecal or EventProc)

    Attributes
    ----------
    tagName : str
        Tag which identifies the generation of information
    """

    def __init__(self, object_name, class_name, module_name):
        self.object_name=object_name
        self.class_name=class_name
        self.tag_name=''

        # make sure process loads this library if it hasn't yet
        Process.add_module(module_name)

        #register this conditions object provider with the process
        Process.declare_conditions_object_provider(self)

    def set_tag(self,newtag) :
        """Set the tag generation of the Conditions

        Parameters
        ----------
        newtag : str
            Tag for generation of conditions
        """

        self.tag_name=newtag

    def __eq__(self,other) :
        """Check if two COPs are the same

        We decide that two COPs are 'equal' if they have the same instance and class names
        
        Parameters
        ----------
        other : ConditionsObjectProvider
            other COP to compare agains
        """

        if not isinstance(other,ConditionsObjectProvider) :
            return NotImplemented

        return (self.object_name == other.object_name and self.class_name == other.class_name)

    def __str__(self) :
        """Stringify this ConditionsObjectProvider, creates a message with all the internal parameters.

        Returns
        -------
        str
            A message with all the parameters and member variables in a human readable format
        """

        msg = "\n  ConditionsObjectProvider(%s of class %s, tag='%s')"%(self.object_name,self.class_name,self.tag_name)
        if len(self.__dict__)>0:
            msg += "\n   Parameters:"
            for k, v in self.__dict__.items():
                msg += "\n    " + str(k) + " : " + str(v)

        return msg

class RandomNumberSeedService(ConditionsObjectProvider):
    """The random number seed service

    This object registers the random number seed service with the process and
    gives some helper functions for configuration.

    Attributes
    ----------
    seedMode : str
        Name of mode of getting random seeds
    """

    def __init__(self) :
        super().__init__('RandomNumberSeedService','framework::RandomNumberSeedService','Framework')
        self.seed_mode = ''
        self.seed=-1 #only used in external mode

        # use run seed mode by default
        self.run()

    def run(self) :
        """Base random number seeds off of the run number"""
        self.seed_mode = 'run'

    def external(self,seed) :
        """Input the master random number seed

        Parameters
        ----------
        seed : int
            Integer to use as master random number seed
        """
        self.seed_mode = 'external'
        self.seed = seed

    def time(self) :
        """Set master random seed based off of time"""
        self.seed_mode = 'time'


class _LogRule:
    """A single pair holding a channel name and the level it should be logged at

    This class should not be used directly, use the helper functions
    in Logger instead to define rule sets.
    """
    def __init__(self, name, level):
        self.name = name
        self.level = level


class Logger:
    """Configure the logging infrastructure of ldmx-sw

    The "severity level" of messages correspond to the following integers.

        - -1: Trace
        - 0  : Debug
        - 1  : Information
        - 2  : Warning
        - 3  : Error
        - 4  : Fatal (reserved for program-ending exceptions)

    Whenever a level is specified, messages for that level and any level above
    it (corresponding to a larger integer) are including when printing out
    the messages.

    Paramters
    ---------
    termLevel: int
        minimum severity level to print to the terminal
    fileLevel: int
        minimum severity level to print to the file
    file_path: str
        path to file to direct logging to (if not provided, don't open a file for logging)
    logRules: List[_LogRule]
        list of custom logging rules that override the default terminal and file levels
    """

    def __init__(self):
        self.term_level = 2 # warnings and above
        self.file_level = 0 # everything
        self.file_path  = '' # don't open file for logging
        self.log_rules = []


    def custom(self, name, level):
        """Add a new custom logging rule to the logger

        We automatically get the event processor's instance name
        if an instance of an event processor is passed.

        Parameters
        ----------
        name: str|EventProcessor
            identification of the logging channel to customize
        level: int
            level (and above) messages to print for that channel
        """

        if isinstance(name, EventProcessor):
            name = name.instance_name
        self.log_rules.append(_LogRule(name, level))

    def trace(self, name):
        """drop the input channel to the trace level"""
        self.custom(name, level = -1)

    def debug(self, name):
        """drop the input channel to the debug level"""
        self.custom(name, level = 0)


    def silence(self, name):
        """raise the input channel to the error-only level"""
        self.custom(name, level = 3)


class Process:
    """Process configuration object

    The python object that stores the necessary parameters for configuring
    a Process for ldmx-app to execute.

    Upon construction, the class-wide reference lastProcess is set
    and the rest of the attributes are set to sensible defaults.

    Parameters
    ----------
    pass_name : str
        Short reference name for this run of the process

    Attributes
    ----------
    lastProcess : Process
        Class-wide reference to the last Process object to be constructed
    max_events : int
        Maximum number events to process.
        If totalEvents is set, this will be ignored.
    minEvents : int
        Index of the first events to process.
        The skipping process is relatively slow, if used for anything outside of debugging
        make a  skim to a new file and then run again rather than use this.
        Note: this skips events of *each* input file, you a single file only.
    maxTriesPerEvent : int
        Maximum number of attempts to make in a row before giving up on an event
        Only used in Production Mode (no input files)
        If totalEvents is set, this will be ignored.
    totalEvents : int
        Number of events we'd like to produce independetly of the number of tries it would take.
        Both max_events and maxTriesPerEvent will be ignored. Be warned about infinite loops!
    run : int
        Run number for this process
    inputFiles : list of strings
        Input files to read in event data from and process
    outputFiles : list of strings
        Output files to write out event data to after processing
    sequence : list of Producers and Analyzers
        List of event processors to pass the event bus objects to
    keep : list of strings
        List of rules to keep or drop objects from the event bus
    libraries : list of strings
        List of libraries to load before attempting to build any processors
    skimDefaultIsKeep : bool
        Flag to say whether to process should by default keep the event or not
    skimRules : list of strings
        List of skimming rules for which processors the process should listen to when deciding whether to keep an event
    logFrequency : int
        Print the event number whenever its modulus with this frequency is zero
    logger : Logger
        configuration for logging system in ldmx-sw
    conditionsGlobalTag : str
        Global tag for the current generation of conditions
    conditionsObjectProviders : list of ConditionsObjectProviders
        List of the sources of calibration and conditions information
    randomNumberSeedService : RandomNumberSeedService
        conditions object that provides random number seeds in a deterministic way

    See Also
    --------
    Producer : one type of event processor
    Analyzer : the other type of event processor
    """

    last_process =None

    def __init__(self, pass_name):

        if ( Process.last_process is not None ) :
            raise Exception( "Process object is already created! You can only create one Process object in a script." )

        self.pass_name=pass_name
        self.max_events=-1
        self.min_events=-1
        self.max_tries_per_event=1
        self.run=-1
        self.input_files=[]
        self.output_files=[]
        self.sequence=[]
        self.keep=[]
        self.libraries=[]
        self.skim_default_is_keep=True
        self.skim_rules=[]
        self.log_frequency=-1
        self.logger = Logger()
        self.compression_setting=9
        self.histogram_file=''
        self.conditions_global_tag='Default'
        self.conditions_object_providers=[]
        self.tree_name = 'LDMX_Events'
        Process.last_process=self

        # needs lastProcess defined to self-register
        self.random_number_seed_service=RandomNumberSeedService()


    def __setattr__(self, key, val):
        logger_remap = {
            'termLogLevel' : 'termLevel',
            'fileLogLevel' : 'fileLevel',
            'logFileName'  : 'file_path'
        }
        if key in logger_remap:
            setattr(self.logger, logger_remap[key], val)
            return
        elif key == 'logFrequency' and val > 0:
            # make sure the Process channel is lowered to info
            # later log rules override earlier ones so we put this
            # at the front of the list so the user could have overwritten
            # this if need be
            # 'Process' needs to match the name given in enableLogging
            # in include/Framework/Process.h
            self.logger.log_rules.insert(0, _LogRule('Process', level=1))
            # fall through to set the key=val

        super().__setattr__(key, val)


    def add_library(lib) :
        """Add a library to the list of dynamically loaded libraries

        A process object must already have been created.

        Parameters
        ----------
        lib : str
            name of library to load 

        Warnings
        --------
        - Will exit the script if a process object hasn't been defined yet.

        Examples
        --------
            addLibrary( 'libSimCore.so' )
        """

        if ( Process.last_process is not None ) :
            Process.last_process.libraries.append( lib )
        else :
            raise Exception( "No Process object defined yet! You need to create a Process before creating any EventProcessors." )

    def add_module(module) :
        """Add a module to the list of dynamically loaded libraries

        A process object must already have been created.

        Parameters
        ----------
        module : str
            Name of module to load as a library

        See Also
        --------
        Process.addLibrary

        Examples
        --------
        You can use this function to load a general module
            addModule('SimCore')

        With the string substitutions that are made, you can
        refer to submodules with cmake, C++, or the library
        syntax. The following calls are all equivalent.
            addModule('Ecal/Event')
            addModule('Ecal::Event')
            addModule('Ecal_Event')
        """

        actual_module_name = module.replace('/','_').replace('::','_')
        Process.add_library('@CMAKE_INSTALL_PREFIX@/lib/lib%s.so'%(actual_module_name))

    def declare_conditions_object_provider(cop):
        """Declare a conditions object provider to be loaded with the process

        A process object must already have been created.

        Parameters
        ----------
        cop : ConditionsObjectProvider
            provider to load with the process

        Warnings
        --------
        - Will exit the script if a process object hasn't been defined yet.
        - Overrides an already declared COP with the passed COP if they are equal
        """

        if ( Process.last_process is not None ) :

            cop.set_tag(Process.last_process.conditions_global_tag)

            # check if the input COP matches one already declared
            #   if it does match, override the already declared one with the passed one
            for index, already_defined_cop in enumerate(Process.last_process.conditions_object_providers) :
                if cop == already_defined_cop :
                    Process.last_process.conditions_object_providers[index] = cop
                    return

            Process.last_process.conditions_object_providers.append( cop )
        else :
            raise Exception( "No Process object defined yet! You need to create a Process before declaring any ConditionsObjectProviders." )

    def set_conditions_global_tag(self,tag) :
        """Set the global tag for all the ConditionsObjectProviders

        Parameters
        ----------
        tag : str
            Global generation tag to pass to all COPs
        """

        self.conditions_global_tag=tag
        for cop in self.conditions_object_providers :
            cop.set_tag(tag)

    def skim_default_is_save(self):
        """Configure the process to by default keep every event."""

        self.skim_default_is_keep=True

    def skim_default_is_drop(self):
        """Configure the process to by default drop (not save) every event."""

        self.skim_default_is_keep=False

    def skim_consider(self,name_pat):
        """Configure the process to listen to processors matching input.

        The list of skim rules has a rather complicated form, so it
        is better to use this helper function.

        Parameters
        ----------
        name_pat : str
            Pattern for the processor instanceNames to match for the Process to listen

        Example
        -------
            ecal_veto = ldmxcfg.Producer( 'ecal_veto' , 'EcalVetoProcessor' )
            # Setup of other parameters for the veto
            p.skim_consider( 'ecal_veto' )

        See Also
        --------
        skimConsiderLabelled

        """

        self.skim_rules.append(name_pat)
        self.skim_rules.append("")

    def skim_consider_labelled(self,name_pat,label_pat):
        """Configure the process to listen to processors matching input.

        The list of skim rules has a rather complicated form, so it
        is better to use this helper function.

        Some uses of ``setStorageHint`` in processors include a "reason"
        for the given storage hint. The input label pattern is check
        on matching this "reason" for the storage hint.

        Parameters
        ----------
        name_pat : str
            Pattern for the processor instanceNames to match for the Process to listen
        label_pat : str
            Pattern for the storage hint reason to match for the Process to listen

        See Also
        --------
        skimConsider

        """
        self.skim_rules.append(name_pat)
        self.skim_rules.append(label_pat)

    def setCompression(self,algorithm,level=9):
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

        self.compression_setting = algorithm*100 + level

    def inputDir(self, indir) :
        """Scan the input directory and make a list of input root files to read from it

        Lists all files ending in '.root' in the input directory (not recursive).
        Extends the inputFiles list by these files.

        Parameters
        ----------
        indir : str
            Path to directory of event files to read in
        """

        import os
        full_path_dir = os.path.realpath(indir)
        self.input_files.extend([ os.path.join(full_path_dir,f)
                for f in os.listdir(full_path_dir)
                if os.path.isfile(os.path.join(full_path_dir,f)) and f.endswith('.root')
                ])

    def parameterDump(self) :
        """Recursively extract all configuration parameters for this process

        Only includes objects somehow attached to the process.
        """

        keys_to_skip = [ 'histograms' , 'libraries' ]

        from LDMX.Framework import histogram as h
        from LDMX.SimCore import simcfg

        def extract(obj):
            """Extract the parameter from the input object"""

            if isinstance(obj,list) :
                return [ extract(o) for o in obj ]
            elif hasattr(obj,'__dict__') :
                params = dict()
                for k in obj.__dict__ :
                    if k not in keys_to_skip :
                        params[k] = extract(obj.__dict__[k])
                return params
            else :
                return obj

        return extract(self)


    def pause(self) :
        """Print this Process and wait for user confirmation to continue

        Prints the process through the print function, and then
        waits for the user to press Enter to continue.
        """

        print(self)
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

        msg = "Process with pass name '%s'"%(self.pass_name)
        if (self.run>0): msg += "\n using run number %d"%(self.run)
        if (self.max_events>0): msg += "\n Maximum events to process: %d"%(self.max_events)
        else: msg += "\n No limit on maximum events to process"
        if (len(self.conditions_object_providers)>0):
            msg += "\n conditionsObjectProviders:\n"
            for cop in self.conditions_object_providers:
                msg+=str(cop)
        msg += "\n Processor sequence:"
        for proc in self.sequence:
            msg += str(proc)
        if len(self.input_files) > 0:
            if len(self.output_files)==len(self.input_files):
                msg += "\n Files:"
                for i in range(0,len(self.input_files)):
                    msg += "\n  '%s' -> '%s'"%(self.input_files[i],self.output_files[i])
            else:
                msg += "\n Input files:"
                for afile in self.input_files:
                    msg += '\n  ' + afile
                if len(self.output_files) > 0:
                    msg += "\n Output file: " + self.output_files[0]
        elif len(self.output_files) > 0:
            msg += "\n Output file: " + self.output_files[0]
        msg += "\n Skim rules:"
        if self.skim_default_is_keep: msg += "\n  Default: keep the event"
        else: msg += "\n  Default: drop the event"
        for i in range(0,len(self.skim_rules)-1,2):
            if self.skim_rules[i+1]=="":
                msg += "\n  Listen to hints from processors with names matching '%s'"%(self.skim_rules[i])
            else:
                msg += "\n  Listen to hints with labels matching '%s' from processors with names matching '%s'"%(self.skim_rules[i+1],self.skim_rules[i])
        if len(self.keep) > 0:
            msg += "\n Rules for keeping previous products:"
            for arule in self.keep:
                msg += '\n  ' + arule
        if len(self.libraries) > 0:
            msg += "\n Shared libraries to load:"
            for afile in set(self.libraries):
                msg += '\n  ' + afile

        return msg

class RunHeaderAna(Analyzer) :
    """                                                                                                                  
    Contains an instance of RunHeaderAnalyzer that
    has already been configured.

    Examples
    --------
        p.sequence.append( ldmxcfg.RunHeaderAna() )
    """

    def __init__(self, name='RunHeaderAnalyzer'):
        super().__init__(name, 'framework::RunHeaderAnalyzer', 'Framework')
