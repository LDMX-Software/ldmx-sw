from ._conditions_object_provider import (
    ConditionsObjectProvider,
    conditions_object_provider,
)


@conditions_object_provider(
    "RandomNumberSeedService", "framework::RandomNumberSeedService", "Framework"
)
class RandomNumberSeedService(ConditionsObjectProvider):
    """The random number seed service

    This object registers the random number seed service with the process and
    gives some helper functions for configuration.

    Attributes
    ----------
    seedMode : str
        Name of mode of getting random seeds, uses run number by default
    seed: int
        integer seed only used in external mode
    """

    seed_node: str = "run"
    seed: int = -1

    def run(self):
        """Base random number seeds off of the run number"""
        self.seed_mode = "run"

    def external(self, seed):
        """Input the master random number seed

        Parameters
        ----------
        seed : int
            Integer to use as master random number seed
        """
        self.seed_mode = "external"
        self.seed = seed

    def time(self):
        """Set master random seed based off of time"""
        self.seed_mode = "time"
