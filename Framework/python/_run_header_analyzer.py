from . import processor


@processor("framework::RunHeaderAnalyzer", "Framework")
class RunHeaderAna(Processor):
    """Contains an instance of RunHeaderAnalyzer that
    has already been configured.

    Examples
    --------
        p.sequence.append( LDMX.Framework.RunHeaderAna() )
    """

    pass
