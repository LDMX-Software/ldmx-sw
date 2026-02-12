from ._parameter_set import parameter_set

@parameter_set
class _LogRule:
    """A single pair holding a channel name and the level it should be logged at

    This class should not be used directly, use the helper functions
    in Logger instead to define rule sets.
    """

    name: str
    level: int


@parameter_set
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
    filePath: str
        path to file to direct logging to (if not provided, don't open a file for logging)
    logRules: list[_LogRule]
        list of custom logging rules that override the default terminal and file levels
    """

    termLevel: int = 2
    fileLevel: int = 0
    filePath: str = ''
    logRules: list[_LogRule] = []


    def custom(self, name, level):
        """Add a new custom logging rule to the logger

        We automatically get the event processor's instance name
        if an instance of an event processor is passed.

        Parameters
        ----------
        name: str or processor
            identification of the logging channel to customize
        level: int
            level (and above) messages to print for that channel
        """

        if hasattr(name, 'instanceName'):
            name = name.instanceName
        self.logRules.append(_LogRule(name, level))


    def trace(self, name):
        """drop the input channel to the trace level"""
        self.custom(name, level = -1)

    def debug(self, name):
        """drop the input channel to the debug level"""
        self.custom(name, level = 0)


    def silence(self, name):
        """raise the input channel to the error-only level"""
        self.custom(name, level = 3)
