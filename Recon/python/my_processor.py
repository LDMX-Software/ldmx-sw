"""Example configuration object for a processor"""

# We need the ldmx configuration package to construct the processor objects
from LDMX.Framework import ldmxcfg

class MyProcessor(ldmxcfg.Producer) :
    """The name is purely conventional to match the C++ class name for clarity

    The line
        super().__init__( name , "recon::MyProcessor" , "Recon" )

    Calls the constructor for ldmxcfg.Producer, which is how we have handles
    on this processor. You need to give the actual C++ class name with 
    namespace(s) as the second entry, and the name of the module the C++ class
    is in as the third entry.

    Any other lines define parameters that are accessible in the C++
    configure method. For example, the line
        self.my_parameter = 20

    defines a integer parameter for this class which can be accessed
    in the configure method with
        int my_parameter = parameters.getParameter<int>("my_parameter");

    Examples
    --------

    Creating the configuration object gives the parameters set
    in __init__.
        myProc = MyProcessor( 'myProc')

    You can also change the parameters after creating this object.
        myProc.my_parameter = 50

    Then you put your processor into the sequence of the process.
        p.sequence.append( myProc )
    """

    def __init__(self, name ):
        super().__init__( name , "recon::MyProcessor" , 'Recon' )

        self.my_parameter = 20
