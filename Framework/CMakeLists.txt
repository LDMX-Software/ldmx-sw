
# declare Event module
module(
  NAME Framework  
  EXECUTABLES src/fire.cxx
  DEPENDENCIES Exception Event DetDescr Tools
  EXTERNAL_DEPENDENCIES ROOT Python
)
