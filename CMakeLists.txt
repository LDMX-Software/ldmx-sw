
# declare Event module
module(
  NAME Framework  
  EXECUTABLES src/fire.cxx
  DEPENDENCIES Event 
  EXTERNAL_DEPENDENCIES ROOT Python
)
