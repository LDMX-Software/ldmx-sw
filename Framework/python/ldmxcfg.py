"""Backwards-compatibility shim for ``from LDMX.Framework import ldmxcfg``.

Re-exports the public API so that existing config files using
``ldmxcfg.Process``, ``ldmxcfg.make_processor``, etc. continue to work.
"""

from ._conditions_object_provider import (
    ConditionsObjectProvider,
    conditions_object_provider,
)
from ._parameter_set import field, parameter_set
from ._process import Process
from ._processor import Processor, make_processor, processor, processor_from_file
from ._run_header_analyzer import RunHeaderAna
