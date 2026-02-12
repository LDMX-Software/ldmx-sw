from . import processor

@processor("framework::RunHeaderAnalyzer", "Framework")
class RunHeaderAna:
    """Contains an instance of RunHeaderAnalyzer that
    has already been configured.

    Examples
    --------
        p.sequence.append( LDMX.Framework.RunHeaderAna() )
    """
