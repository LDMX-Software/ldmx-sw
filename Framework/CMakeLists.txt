# declare Event module
module(
  NAME Framework
  EXTRA_LINK_LIBRARIES dl
  EXECUTABLES src/ldmx-app.cxx
  DEPENDENCIES Event
  EXTERNAL_DEPENDENCIES ROOT Python
)
