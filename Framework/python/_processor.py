from ._parameter_set import parameter_set, field
from . import _register, _histogram


def processor_post_init(self):
    """necessary post_init for all processors

    we make sure the module this processor belongs to
    is registered to be loaded and it has an instance_name
    if it was not defined by the user
    """

    _register.library(self.module_name)


def histogram(
        self, name,
        xlabel = '', xbins = 1, xmin = None, xmax = None,
        ylabel = '', ybins = None, ymin = None, ymax = None,
        weighted = False):
    """declare a histogram for a processor to fill

    If xmin and xmax are not provided, bins is assumed to be
    the bin edges on the x-axis. If they are both provided,
    bins is assumed to be the number of bins on the x-axis.

    If ybins is None, then the histogram is assumed to be 1D
    while if ybins is not None, then the histogram is assumed
    to be 2D.

    Parameters
    ----------
    name : str
        variable name of histogram
    xlabel : str
        title of x-axis of histogram
    xbins : int OR list of floats OR list of str
        Number of bins on x-axis OR bin edges on x-axis OR string categories
    xmin : float
        Minimum edge of bins on x-axis
    xmax : float
        Maximum edge of bins on x-axis
    ylabel : str
        title of y-axis of histogram
    ybins : int OR list of floats OR list of str
        Number of bins on y-axis OR list of bin edges on y-axis OR string categories
    ymin : float
        Minimum edge of bins on y-axis
    ymax : float
        Maximum edge of bins on y-axis
    weighted: bool
        whether to keep track of sum of squared weights

    See Also
    --------
    LDMX.Framework.histogram.histogram : histogram configuration object
    """

    the_x_bins = xbins
    if xmin is not None and xmax is not None :
        the_x_bins = _histogram.uniform_binning(xbins,xmin,xmax)

    if ybins is None:
        # 1D histogram
        self.histograms.append(_histogram.histogram(name, xlabel,the_x_bins, weighted=weighted))
    else:
        # 2D histogram
        the_y_bins = ybins
        if ymin is not None and ymax is not None :
            the_y_bins = _histogram.uniform_binning(ybins,ymin,ymax)

        self.histograms.append(
                h.histogram(name, xlabel,the_x_bins, ylabel, the_y_bins, weighted=weighted)
        )


def processor(class_name: str, module_name: str):
    """declare a processor configuration class

    Parameters
    ----------
    class_name: str
        fully-specified C++ class name including namespace
    module_name: str
        name of library that the processor belongs to
    """

    return parameter_set(
        post_init = processor_post_init,
        class_name = class_name,
        module_name = module_name,
        instance_name = class_name,
        histograms = [],
        helpers = [('histogram',histogram)],
    )


def processor_from_file(
        cls, source_file, 
        class_name = None, needs = [],
        instance_name = None, compile_notice = True,
        **config_kwargs):
    """Construct an event processor "in place" from the passed source file

    Since Framework dynamically loads libraries containing processors after
    the python script has been fully run, we can compile a single-file processor
    into its own library that can then be loaded and run. This function puts
    the library next to the source file and only re-compiles if the source file's
    last modified time is newer than the library (or the library does not exist).

    Note
    ----
    Developing processors in this way is incredible inefficient, especially since it
    does not allow for code to be well organized and split across many files nor does it
    allow for two processors to share common code.
    If you find yourself defining more than one `class` within your new C++ processor,
    it is highly recommended to transition your workflow to including your processor as a
    part of ldmx-sw so that it can fully benefit from a build system.

    Parameters
    ----------
    source_file: str | Path
        path to source file to build into a processor (can be relative to where config is being run)
    class_name: str, default is name of source file
        name of C++ class that is the processor
        defaults to the name of the source file without an extension
    needs: list[str]
        Names of libraries that should be linked to the compiled processor in addition to 'Framework'
        which is linked be default.
        For example, one can gain access to the detector ID infrastructure with 'DetDescr' or the
        Ecal Event classes with 'Ecal_Event' (or 'Ecal/Event', or 'Ecal::Event')
    instance_name: str, default is class_name
        name to give to instance of this C++ processor
    compile_notice: bool, default is True
        print a notice when compilation is triggered
    config_kwargs: dict[str, Any]
        configuration parameters to give to the processor

    Examples
    --------
    A basic walkthrough is available online. https://ldmx-software.github.io/analysis/ldmx-sw.html
    
    If `MyAnalyzer.cxx` contains the class `MyAnalyzer`, then we can put

        p.sequence = [ ldmxcfg.Analyzer.from_file('MyAnalyzer.cxx') ]

    In our config script to run the analyzer on its own in the sequence.
    This default configuration only links the Framework library and so the analyzer
    would only be able to access the Framework and event objects.
    If you needed another library (for example, the 'DetDescr' library has the ID classes),
    one can also

        p.sequence = [ ldmxcfg.Analyzer.from_file('MyAnalyzer.cxx', needs = ['DetDescr']) ]

    To inform the compiler that it should link your analyzer with the 'DetDescr' library.
    **No removal of the library is done** so if you change the `needs` or some other parameter
    to `from_file` you should also remove the library file (`*.so`) before attempting to re-run.

    Returns
    -------
    EventProcessor
        built from the C++ source file and configured with the passed arguments
    """

    if not isinstance(source_file, Path):
        source_file = Path(source_file)
    if not source_file.is_file():
        raise ValueError(f'{source_file} is not accessible.')

    src = source_file.resolve()

    if class_name is None:
        # assume class name is name of file (no extension) if not provided
        class_name = src.stem

    if instance_name is None:
        # use class name for instance name if not provided
        instance_name = class_name

    lib = src.parent / f'lib{src.stem}.so'
    if not lib.is_file() or src.stat().st_mtime > lib.stat().st_mtime:
        if compile_notice:
            print(
                f'Processor source file {src} is newer than its compiled library {lib}'
                ' (or library does not exist), recompiling...'
            )
        import subprocess
        libs_to_link = set(['Framework']+needs)
        cmd = [
            'g++', '-std=c++20', '-fPIC', '-shared', # construct a shared library for dynamic loading
            '-o', str(lib), str(src), # define output file and input source file
        ]+[
            f'-l{lib}' for lib in libs_to_link
        ]+[
            '-I/usr/local/include/root', # include ROOT's non-system headers
            '-I@CMAKE_INSTALL_PREFIX@/include', # include ldmx-sw headers (if non-system)
            '-L@CMAKE_INSTALL_PREFIX@/lib', # include ldmx-sw libs (if non-system)
        ]
        if compile_notice:
            print(*cmd)
        subprocess.run(cmd, check=True)
        if compile_notice:
            print(f'done compiling {src}')

    # TODO: figure out how to dynamically create a dataclass
    instance = cls(instance_name, class_name, str(lib))
    for cfg_name, cfg_val in config_kwargs.items():
        setattr(instance, cfg_name, cfg_val)

    # load any dependency libraries at runtime as well
    for mod in needs:
        _register.library(mod)

    return instance
