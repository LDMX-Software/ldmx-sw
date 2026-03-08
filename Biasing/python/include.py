"""Include the Biasing module"""

def library() :
    """Attach the name of Biasing library to the process"""
    from LDMX.Framework.ldmxcfg import Process
    Process.add_library('@CMAKE_INSTALL_PREFIX@/lib/libBiasing.so')
