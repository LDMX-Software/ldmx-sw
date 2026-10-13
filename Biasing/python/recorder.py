"""configuration for recording user actions"""

from LDMX.SimCore.user_actions import UserAction, user_action


@user_action("biasing::VertexRecorder", "Biasing")
class VertexRecorder(UserAction):
    """Record the kinematics of particles entering and leaving each
    interaction vertex.

    For every interaction of a primary particle in the configured process,
    the incoming particle and all outgoing products are recorded into the
    event as "VertexRecords".

    Attributes
    ----------
    region : str
        Only record vertices occurring in this Geant4 region (e.g. the
        target). Empty string (default) records vertices anywhere.
    process : str
        The Geant4 process name to record (e.g. "eBrem", "photonNuclear",
        "conv"). Empty string (default) records any process.
    """

    region: str = ""
    process: str = ""
