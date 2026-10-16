"""register objects with global variables for later use by the configuration"""


def library_name(name: str) -> str:
    """deduce the name of a library from how the user referred to it

    The cmake, C++, and library syntax for referring to a submodule are all
    accepted, so 'Ecal/Event', 'Ecal::Event', and 'Ecal_Event' all lead to
    the same library name.

    Parameters
    ----------
    name : str
        name of module whose library we want
    """

    return name.replace("/", "_").replace("::", "_")


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
        full_path = f"@CMAKE_INSTALL_PREFIX@/lib/lib{library_name(name)}.so"

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

    from ._process import Process

    if Process.last_process is None:
        # Process not created yet,
        # put into this functions registry
        conditions_object_provider.__registry__.append(cop)
    else:
        # Process is created, append directly
        Process.last_process._declare_conditions_object_provider(cop)


conditions_object_provider.__registry__ = []
