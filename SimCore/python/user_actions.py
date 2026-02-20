"""Internal configuration module for UserActions"""

from LDMX.Framework import _register, field, parameter_set


class UserAction:
    pass


def user_action(class_name: str, module_name: str = "SimCore"):
    return parameter_set(
        class_name=field(default=class_name, init=False),
        module_name=field(default=module_name, init=False),
        instance_name=class_name,
        post_init=lambda self: _register.library(self.module_name),
        required_base=UserAction,
    )
