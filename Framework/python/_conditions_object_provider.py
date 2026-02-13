from ._parameter_set import parameter_set
from . import _register

def conditions_object_provider_post_init(self):
    _register.library(self.module_name)
    self.__dict__['__is_conditions_object_provider__'] = True
    _register.conditions_object_provider(self)


def conditions_object_provider__eq__(self, other):
    """Check if two COPs are the same

    We decide that two COPs are 'equal' if they have the same instance and class names
    because in that situation the two COPs would class during runtime even if they
    technically provide different objects.
    
    Parameters
    ----------
    other :
        other COP to compare against
    """

    if not getattr(other, '__is_conditions_object_provider__', False):
        return NotImplemented

    return (self.object_name == other.object_name and self.class_name == other.class_name)


def conditions_object_provider(object_name: str, class_name: str, module_name: str):
    return parameter_set(
        post_init = conditions_object_provider_post_init,
        object_name = object_name,
        class_name = class_name,
        module_name = module_name,
        tag_name = '',
        helpers = [('__eq__', conditions_object_provider__eq__)]
    )
