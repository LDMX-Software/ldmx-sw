"""Wrap an uproot file for some extra help plotting"""

import uproot

class File :
    """File entry in Differ object

    Parameters
    ----------
    filepath : str or pathlib.Path
        path specifying ROOT file to open for reading
    name : str
        name for labeling histograms plotted from this file
    colmod : function with str argument returning str
        modify an input column name to align with the columns in this file
        can be used for example to change the pass name
    hist_kwargs : dict
        dictionary providing extra detail for the matplotlib hist call
        helpful for specifying style options and other defaults for hists from this file
    open_kwargs : dict
        all other key-word arguments are passed to uproot.open
    """

    def __init__(self, filepath, name, colmod = None, hist_kwargs = dict(), **open_kwargs) :
        self.__file = uproot.open(filepath, **open_kwargs)
        self.__name = name
        self.__colmod = colmod
        self.__df = None

        if 'histtype' not in hist_kwargs :
            hist_kwargs['histtype'] = 'step'
        if 'linewidth' not in hist_kwargs :
            hist_kwargs['linewidth'] = 2
        if 'label' not in hist_kwargs :
            hist_kwargs['label'] = self.__name
        if 'bins' not in hist_kwargs :
            hist_kwargs['bins'] = 'auto'
            
        self.__hist_kwargs = hist_kwargs


    def keys(self, *args, **kwargs) :
        """Callback into uproot keys

        Helpful for exploring the file when trying to decide
        what to plot within a notebook
        """
        return self.__file.keys(*args, **kwargs)
    
    def events(self, **kwargs) :
        """Callback for retrieving a full in-memory data frame of the events
        
        All key-word arguments are passed to the uproot.arrays method.
        
        We change the default 'library' to be pandas which can be overridden
        by a user if desired.
        """

        if 'library' not in kwargs :
            kwargs['library'] = 'pd'
        return self.__file['LDMX_Events'].arrays(**kwargs)
    
    def load(self, manipulation = None, **kwargs) :
        """Instead of giving the events data frame to the caller,
        we store the dataframe here for later batch processing
        
        manipulation is a function operating on the loaded dataframe
        which is there for people to rename columns, calculate new
        columns, etc...
        
        All the kwargs are simply provided to events for selecting
        the branches of LDMX_Events to load into memory.
        """
        self.__df = self.events(**kwargs)
        if manipulation is not None :
            manipulation(self.__df)
    
    def manipulate(self, manipulation) :
        """Apply the passed manipulation to the dataframe"""
        manipulation(self.__df)
    
    def plot1d(self, ax, obj_name, **hist_kwargs) :
        """Plot the input uproot object as a histogram on the input axes

        If obj_name is not a str, we assume that it is a callable and will
        provide the in-memory dataframe. An exception is thrown if the dataframe
        has not been loaded.
        
        If the dataframe of events has been loaded and the obj_name is a
        member of that dataframe, we use that dataframe to fill the histogram.
        
        If the uproot_obj is already a histogram we import its values and use
        them directly. If the uproot_obj is a TBranch, then we pull its values
        into memory and fill the histogram.

        The input 'obj_name' is transformed by __colmod if that member is set.
        """

        for k, v in self.__hist_kwargs :
            if k not in hist_kwargs :
                hist_kwargs[k] = v
              
        if not isinstance(obj_name, str) :
                if self.__df is None :
                        raise KeyError('Cannot use dynamic value calculations without loading the dataframe.')
                return ax.hist(obj_name(self.__df), **hist_kwargs)

        if self.__df is not None and obj_name in self.__df :
                return ax.hist(self.__df[obj_name], **hist_kwargs)
        
        if self.__colmod is not None :
            obj_name = self.__colmod(obj_name)

        obj = self.__file[obj_name]

        if issubclass(type(obj), uproot.behaviors.TH1.Histogram) :
            edges = obj.axis('x').edges()
            dim = len(edges.shape)
            if dim > 1 :
                raise KeyError(f'Attempted to do a 1D plot of a {dim} dimension histogram.')
            # overwrite bins and weights with what the serialized histogram has
            hist_kwargs['bins'] = edges
            hist_kwargs['weights'] = obj.values()
            return ax.hist((edges[1:]+edges[:-1])/2, **hist_kwargs)
        else :
            return ax.hist(obj.array(library='pd').values, **hist_kwargs)
