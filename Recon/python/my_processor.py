"""Example configuration object for a processor"""

from LDMX.Framework import Processor, processor


@processor("recon::MyProcessor", "Recon")
class MyProcessor(Processor):
    """The name is purely conventional to match the C++ class name for clarity

    The @processor decorator and Processor base class handle the registration
    with the framework. You need to give the actual C++ class name with
    namespace(s) as the first argument, and the name of the module the C++ class
    is in as the second argument.

    Any class-level attributes define parameters that are accessible in the C++
    configure method. For example, the attribute
        my_parameter: int = 20

    defines an integer parameter for this class which can be accessed
    in the configure method with
        int my_parameter = parameters.get<int>("my_parameter");

    Examples
    --------

    Creating the configuration object gives the parameters set
    in the class definition.
        myProc = MyProcessor( instance_name='myProc')

    You can also change the parameters after creating this object.
        myProc.my_parameter = 50

    Then you put your processor into the sequence of the process.
        p.sequence.append( myProc )
    """

    my_parameter: int = 20
    ecal_rechits_passname: str = ""
    ecal_rec_hits_event_passname: str = ""
