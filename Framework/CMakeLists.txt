# declare Event module
module(
  NAME Framework  
  EXECUTABLES src/ldmx-app.cxx
  DEPENDENCIES Event DetDescr
  EXTERNAL_DEPENDENCIES ROOT Python
)
