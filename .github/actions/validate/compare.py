
import os
import sys
import ROOT
import subprocess
ROOT.gROOT.SetBatch(1)
ROOT.gStyle.SetOptStat(0)

def flatten(l) :
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
    """A root file with histograms that we want to be styled in same way"""

    def __init__(self, f, name, color, fill) :
        self.__file = ROOT.TFile.Open(f)
        self.__name = name
        self.__color = color
        self.__fill = fill

    def list_histograms(self) :
        return flatten(self.__file.GetListOfKeys())

    def get(self, hist_key) :
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

def compare(gold_f, gold_label, test_f, test_label) :
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
            print(e)
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
