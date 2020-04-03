
# declare Event module
module(
  NAME Framework  
  EXECUTABLES src/ldmx-app.cxx
  DEPENDENCIES Exception Event DetDescr Tools
  EXTERNAL_DEPENDENCIES ROOT Python
)
