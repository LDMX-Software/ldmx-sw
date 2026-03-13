import types
from pathlib import Path

from . import _histogram, _register
from ._parameter_set import field, parameter_set


@parameter_set
class Processor:
    instance_name: str
    histograms: list[_histogram.Histogram] = []

    def __post_init__(self):
        """necessary post_init for all processors

        we make sure the module this processor belongs to
        is registered to be loaded
        """
        _register.library(self.module_name)

    def histogram(
        self,
        name,
        xlabel="",
        xbins=1,
        xmin=None,
        xmax=None,
        ylabel="",
        ybins=None,
        ymin=None,
        ymax=None,
        weighted=False,
    ):
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
        if xmin is not None and xmax is not None:
            the_x_bins = _histogram.uniform_binning(xbins, xmin, xmax)

        if ybins is None:
            # 1D histogram
            self.histograms.append(
                _histogram.Histogram(name, xlabel, the_x_bins, weighted=weighted)
            )
        else:
            # 2D histogram
            the_y_bins = ybins
            if ymin is not None and ymax is not None:
                the_y_bins = _histogram.uniform_binning(ybins, ymin, ymax)

            self.histograms.append(
                _histogram.Histogram(
                    name, xlabel, the_x_bins, ylabel, the_y_bins, weighted=weighted
                )
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
        class_name=field(default=class_name, init=False),
        module_name=field(default=module_name, init=False),
        instance_name=class_name,
        required_base=Processor,
    )


def make_processor(name: str, class_name: str, module_name: str, **kwargs):
    """make a processor without having a class to decorate

    like the make_dataclass function, this is purely a convenience to
    avoid some extra boilerplate (and for use when constructing a processor
    from source below).

    Parameters
    ----------
    name: str
        name of the processor
    class_name: str
        fully-specified name of class in C++
    module_name: str
        library to load that the processor is in
    kwargs: dict[str,Any]
        extra key-word arguments assumed to be parameters
        to pass to C++. These parameters DO NOT undergo
        any type checking.
    """

    # dynamically create a new_class by name with the Processor base
    # and call our decorator on it
    cls = parameter_set(
        class_name=class_name,
        module_name=module_name,
        instance_name=name,
        required_base=Processor,
        **kwargs,
    )(types.new_class(name, (Processor,)))
    return cls()


def processor_from_file(
    source_file,
    class_name=None,
    needs=None,
    instance_name=None,
    compile_notice=True,
    **config_kwargs,
):
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
    it is highly recommended to transition your workflow to including your processor as
    a part of ldmx-sw so that it can fully benefit from a build system.

    Parameters
    ----------
    source_file: str | Path
        path to source file to build into a processor
        (can be relative to where config is being run)
    class_name: str, default is name of source file
        name of C++ class that is the processor
        defaults to the name of the source file without an extension
    needs: list[str]
        Names of libraries that should be linked to the compiled processor in addition
        to 'Framework' which is linked be default.
        For example, one can gain access to the detector ID infrastructure with
        'DetDescr' or the Ecal Event classes with 'Ecal_Event'
        (or 'Ecal/Event', or 'Ecal::Event')
    instance_name: str, default is class_name
        name to give to instance of this C++ processor
    compile_notice: bool, default is True
        print a notice when compilation is triggered
    config_kwargs: dict[str, Any]
        configuration parameters to give to the processor

    Examples
    --------
    A basic walkthrough is available online.
    https://ldmx-software.github.io/analysis/ldmx-sw.html

    If `MyAnalyzer.cxx` contains the class `MyAnalyzer`, then we can put

        p.sequence = [ ldmxcfg.processor_from_file('MyAnalyzer.cxx') ]

    In our config script to run the analyzer on its own in the sequence.
    This default configuration only links the Framework library and so the analyzer
    would only be able to access the Framework and event objects.
    If you needed another library (for example, the 'DetDescr' library has
    the ID classes), one can also

        p.sequence = [
            ldmxcfg.processor_from_file('MyAnalyzer.cxx', needs = ['DetDescr'])
        ]

    To inform the compiler that it should link your analyzer with the 'DetDescr'
    library. **No removal of the library is done** so if you change the `needs`
    or some other parameter you should also remove the library file (`*.so`)
    before attempting to re-run.

    Returns
    -------
    Processor
        built from the C++ source file and configured with the passed arguments
    """

    if needs is None:
        needs = []
    if not isinstance(source_file, Path):
        source_file = Path(source_file)
    if not source_file.is_file():
        raise ValueError(f"{source_file} is not accessible.")

    src = source_file.resolve()

    if class_name is None:
        # assume class name is name of file (no extension) if not provided
        class_name = src.stem

    if instance_name is None:
        # use class name for instance name if not provided
        instance_name = class_name

    lib = src.parent / f"lib{src.stem}.so"
    if not lib.is_file() or src.stat().st_mtime > lib.stat().st_mtime:
        if compile_notice:
            print(
                f"Processor source file {src} is newer than its compiled library {lib}"
                " (or library does not exist), recompiling..."
            )
        import subprocess

        libs_to_link = {"Framework", *needs}
        # this compilation command was created through trial and error
        # we compile a shared library with dynamic loading under the C++20 standard
        # linking the requested libraries to our newly built one
        # and including ROOT's non-sytem headers, ldmx-sw headers (if non system)
        # and ldmx-sw libraries (if non-system)
        cmd = (
            [
                "g++",
                "-std=c++20",
                "-fPIC",
                "-shared",
                "-o",
                str(lib),
                str(src),
            ]
            + [f"-l{lib}" for lib in libs_to_link]
            + [
                "-I/usr/local/include/root",
                "-I@CMAKE_INSTALL_PREFIX@/include",
                "-L@CMAKE_INSTALL_PREFIX@/lib",
            ]
        )
        if compile_notice:
            print(*cmd)
        subprocess.run(cmd, check=True)
        if compile_notice:
            print(f"done compiling {src}")

    instance = make_processor(instance_name, class_name, str(lib), **config_kwargs)

    # load any dependency libraries at runtime as well
    for mod in needs:
        _register.library(mod)

    return instance
