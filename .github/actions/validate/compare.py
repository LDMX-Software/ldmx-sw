"""Script to compare two histogram files by 
overlaying histograms with the same key
"""

import os
import sys
import ROOT
import subprocess
ROOT.gROOT.SetBatch(1)
ROOT.gStyle.SetOptStat(0)

def flatten(l) :
    """Get list of all (non-directory) objects in ROOT file with
    the full path to their location in the file.

    This function is recursive so that we can enter into an
    (almost) arbitrary number of subdirectories.

    For each key in the input list we check if it is a folder
    (directory). If it is, then we get the list of objects inside
    that folder and call this function again. Otherwise, we go
    backwards up the chain of mother directories of the object
    to get its full path in the file and add this full path to
    the output "flattened" list.

    Parameters
    ----------
    l : list[TKey]
        list of keys to start flattening

    Examples
    --------
    To get the full list of (non-directory) objects in a file, do
        
        rf = ROOT.TFile('my_file.root')
        full_list = flatten(rf.GetListOfKeys())
    """
    flat_l = [ ]
    for k in l :
        if k.IsFolder() :
            # recurse into subdirectory
            d = k.GetFile().GetDirectory(k.GetName())
            flat_l.extend(flatten(d.GetListOfKeys()))
        else :
            # get full path of object relative to file
            fp = k.GetName()
            m = k.GetMotherDir()
            while m != k.GetFile() :
                fp = m.GetName() + '/' + fp
                m = m.GetMotherDir()
            flat_l.append(fp)

    return flat_l

class HistogramFile() :
    """A root file with histograms that we want to be styled in same way

    This class is not very complicated and is simply here to do two things.

    1) Obtain the full list of objects in the input file using flatten.
    2) Style all of the histograms retrieved from the file in the same way.

    Parameters
    ----------
    f : str
        Name of file to open
    name : str
        Name to label the histograms that come out of this file
    color: int
        ROOT color to color these histograms
    fill : bool
        Should these histograms be filled? (Yes == True)
    """

    def __init__(self, f, name, color, fill) :
        self.__file = ROOT.TFile.Open(f)
        self.__name = name
        self.__color = color
        self.__fill = fill

    def list_histograms(self) :
        """List all of the histograms in this file.

        We use flatten here, so technically this lists all
        of the objects in the file, but we assume we are only
        comparing files with histograms in them.
        """

        return flatten(self.__file.GetListOfKeys())

    def get(self, hist_key) :
        """Get a histogram from this file

        After retrieving the histogram (and checking that it was retrieved successfully),
        we style the histogram by setting the name, color, and fill attributes.

        We set the title of the histogram to the name of the histogram file
        so that the name appears in the legend. We assume the x-axis of the histogram
        is defined to contain the helpful information about what the histogram is.
        """

        h = self.__file.Get(hist_key)
        if 'TH' not in h.__class__.__name__ :
            raise AttributeError(f'{hist_key} does not exist in {self.__file.GetName()}')
        h.SetTitle(f'{self.__name}')
        h.SetLineColor(self.__color)
        h.SetLineWidth(2)
        h.SetMarkerColor(self.__color)
        if self.__fill :
            h.SetFillColor(self.__color)
        return h

def print_error(msg) :
    """Use GitHub workflow command to print errors so that they don't get lost in the logs"""
    print('::error::',msg)

def compare(gold_f, gold_label, test_f, test_label) :
    """Compare two histogram files

    This is the main function of this script.
    The four inputs to this function are the four inputs
    on the command line (in the same order).

    The golden histograms are styled red and filled
    while the test histograms are blue and not filled.

    The golden file decides the list of histograms.
    This means if the test file adds new histograms,
    they will not be compared against anything.

    Each pair of histograms that are plotted are also
    compared using the KS test (TH1::KolmogorovTest) including
    both the underflow and overflow bins with a limit of 0.99.
    This limit can be loosely interpreted as a 99% chance that
    the two histograms are filled from the same distribution.

    If the pair of histograms fail this KS test, their plot
    will be put into the 'fail' directory. If they pass,
    their plot will be put into the 'pass' directory.

    Parameters
    ----------
    gold_f : str
        Name of file with "golden" histograms in it
    gold_label : str
        The label used in the histogram legends for the "golden" histograms
    test_f : str
        Name of file with test histograms in it
    test_label : str
        The label used in the histogram legends for the test histograms
    """

    gold = HistogramFile(gold_f,gold_label,ROOT.kRed ,True )
    test = HistogramFile(test_f,test_label,ROOT.kBlue,False)

    c = ROOT.TCanvas()
    c.SetLogy()

    os.makedirs(f'plots/pass',exist_ok=True)
    os.makedirs(f'plots/fail',exist_ok=True)
    
    for key in gold.list_histograms() :
        try :
            gold_h = gold.get(key)
            test_h = test.get(key)
        except AttributeError as e:
            print_error(e)
            continue

        empty_gold = (gold_h.GetEntries() == 0)
        empty_test = (test_h.GetEntries() == 0)

        sub_dir = 'pass'
        if empty_gold and empty_test :
            # both empty, call this a pass
            sub_dir = 'pass'
        elif not empty_gold and not empty_test :
            if gold_h.KolmogorovTest(test_h,'UO') < 0.99 :
                # both non-empty and they fail the KS test
                sub_dir = 'fail'
            else :
                sub_dir = 'pass'
        else :
            # one empty and other non-empty
            sub_dir = 'fail'

        gold_h.Draw()
        test_h.Draw('same')
    
        c.BuildLegend()
        c.SaveAs(f'plots/{sub_dir}/{key.replace("/","_").replace(":","_")}.pdf')

if __name__ == '__main__' :
    import sys
    compare(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
