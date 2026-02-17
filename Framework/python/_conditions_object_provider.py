from . import _register
from ._parameter_set import field, parameter_set


class ConditionsObjectProvider:
    def __post_init__(self):
        _register.library(self.module_name)
        _register.conditions_object_provider(self)

    def __eq__(self, other):
        """Check if two COPs are the same

        We decide that two COPs are 'equal' if they have the same instance and class names
        because in that situation the two COPs would class during runtime even if they
        technically provide different objects.

        Parameters
        ----------
        other :
            other COP to compare against
        """

        if not isinstance(other, ConditionsObjectProvider):
            return NotImplemented

        return (
            self.object_name == other.object_name
            and self.class_name == other.class_name
        )


def conditions_object_provider(object_name: str, class_name: str, module_name: str):
    return parameter_set(
        object_name=field(default=object_name, init=False),
        class_name=field(default=class_name, init=False),
        module_name=field(default=module_name, init=False),
        tag_name="",
        required_base=ConditionsObjectProvider,
    )
