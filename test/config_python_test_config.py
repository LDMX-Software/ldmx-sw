
from LDMX.Framework import ldmxcfg

class TestProcessor(ldmxcfg.Producer):
  """Configuration for the producer defined in ConfigurePythonTest.

  Attributes
  ----------
  test_int : int
    Int test parameter.
  test_double : double
    Float test parameter.
  test_string : str
    string test parameter.
  test_int_vec : list(int)
    List of ints test parameter.
  test_double_vec : list(double)
    List of doubles test parameter.
  test_string_vec : list(str)
    List of strings test parameter.
  test_dict : dict
    Dictionary test parameter.
  """
  
  def __init__(self): 
    super().__init__('test_instance', 'framework::test::TestConfig', 'Framework')
    
    self.test_int = 9
    self.test_double = 7.7
    self.test_string = 'Yay!'
    self.test_int_vec = [ 1 , 2 , 3 ]
    self.test_dict = { 'one' : 1, 'two' : 2.0 }
    self.test_double_vec = [ 0.1 , 0.2 , 0.3 ]
    self.test_string_vec = [ 'first' , 'second' , 'third' ]
    self.test_2dlist = [ [ 11, 12, 13], [21, 22], [31,32,33,34]]


# Create a process
p = ldmxcfg.Process( 'test' )

# Specify the input files
p.inputFiles = [ 'input1' , 'input2' ]

# Specify whether events should be kept by default
p.skimDefaultIsKeep = False

# But the processor pipeline
p.sequence = [ TestProcessor() ]
