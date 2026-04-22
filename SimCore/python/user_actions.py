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


@user_action("simcore::PhotonuclearTracker")
class PhotonuclearTracker(UserAction):
    """Configuration for tracking detailed photonuclear interaction information

    This UserAction captures comprehensive details about photonuclear (PN)
    interactions during simulation, including incident photon kinematics,
    target nucleus information, immediate secondaries, and descendant genealogy.

    The collected interactions are saved to the event as "PhotonuclearInteractions".

    Attributes
    ----------
    enabled : bool, optional
        Enable or disable tracking (default: True)
    """

    enabled: bool = True
