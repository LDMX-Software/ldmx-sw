
from LDMX.Framework import Process, processor, parameter_set, field

@parameter_set
class MyParameters:
    one: int = 1
    two: float = 2.0


@processor("framework::test::TestConfig", "Framework")
class TestProcessor:
    instanceName: str = "test_instance"
    test_int: int = 9
    test_double: float = 7.7
    test_string: str = "Yay!"
    test_int_vec: list[int] = [ 1 , 2 , 3 ]
    test_dict: MyParameters = field(default_factory = MyParameters)
    test_double_vec: list[float] = [ 0.1 , 0.2 , 0.3 ]
    test_string_vec: list[str] = [ 'first' , 'second' , 'third' ]
    test_2dlist: list[int,int] = [ [ 11, 12, 13], [21, 22], [31,32,33,34]]
    test_collection_passname: str = ''
    test_object_passname: str = ''
    veto_test_object_passname: str = ''
    tenth_event_passname: str = ''
    event_index_passname: str = ''


# Create a process
p = Process( 'test' )

# Specify the input files
p.input_files = [ 'input1' , 'input2' ]

# Specify whether events should be kept by default
p.skim_default_is_keep = False

# But the processor pipeline
p.sequence = [ TestProcessor() ]
