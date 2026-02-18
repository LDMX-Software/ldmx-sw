"""register objects with global variables for later use by the configuration"""


def library(name: str):
    """register a new library to be loaded at run time

    Parameters
    ----------
    name : str
        Name of module to load as a library or
        full path to library to load

    Examples
    --------
    You can use this function to load a general module
        register.library('SimCore')

    With the string substitutions that are made, you can
    refer to submodules with cmake, C++, or the library
    syntax. The following calls are all equivalent.
        register.library('Ecal/Event')
        register.library('Ecal::Event')
        register.library('Ecal_Event')

    If you have the full path to a library already deduced,
    you can provide that as well.

        register.library('/full/path/to/libMyLibrary.so')

    String substitutions and full-path deducation are only
    done if the passed string does not end in '.so'
    """

    full_path = name
    if not full_path.endswith(".so"):
        actual_module_name = name.replace("/", "_").replace("::", "_")
        full_path = f"@CMAKE_INSTALL_PREFIX@/lib/lib{actual_module_name}.so"

    from ._process import Process

    if Process.last_process is None:
        # Process not created yet,
        # put into this functions registry
        library.__registry__.append(full_path)
    else:
        # Process is created, append directly
        Process.last_process.libraries.append(full_path)


library.__registry__ = []


def conditions_object_provider(cop):
    """register a conditions object provider to be included in the run

    Parameters
    ----------
    cop:
        a conditions object provider object to be included in the run
    """

    print('registring', cop)

    from ._process import Process

    if Process.last_process is None:
        print('last_process is None')
        # Process not created yet,
        # put into this functions registry
        conditions_object_provider.__registry__.append(cop)
    else:
        print('append directly')
        # Process is created, append directly
        Process.last_process._declare_conditions_object_provider(cop)


conditions_object_provider.__registry__ = []
