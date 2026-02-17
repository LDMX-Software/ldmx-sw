"""Internal configuration module for simulation objects

The simulation requires a lot more detailed configuration than
the other processors, so we have two extra objects that require
their own python classes.
"""
from LDMX.Framework import parameter_set, _register


def user_action(class_name: str, module_name: str = 'SimCore'):
    return parameter_set(
        class_name = class_name,
        module_name = module_name,
        instance_name = class_name,
        post_init = lambda self: _register.library(self.module_name)
    )




def sensitive_detector(class_name: str, module_name: str = 'SimCore_SDs'):
    """Configuration for a sensitive detector we want to load

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a PrimaryGenerator
    class_name : str
        Name of C++ class that this PrimaryGenerator should be
    module_name : str
        Name of C++ library that this primary generator is compiled into
    """
    return parameter_set(
        class_name = class_name,
        instance_name = class_name,
        module_name = module_name,
        post_init = lambda self: _register.library(self.module_name)
    )


def photo_nuclear_module(class_name: str, module_name: str = 'SimCore_PhotoNuclearModels'):
    """Configuration for a photonuclear model that we want to load

    Parameters
    ----------
    instance_name : str
        Unique name for this particular instance of a PhotoNuclear Model
    class_name : str
        Name of C++ class that this PhotoNuclear model should be
    module_name : str
        Name of C++ library that this PhotoNuclear model is compiled into
    """
    return parameter_set(
        class_name = class_name,
        instance_name = class_name,
        module_name = module_name,
        post_init = lambda self: _register.library(self.module_name)
    )
