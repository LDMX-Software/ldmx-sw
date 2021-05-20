
import os
import sys
import ROOT
import subprocess
ROOT.gROOT.SetBatch(1)
ROOT.gStyle.SetOptStat(0)

def sample_id_from_path(config_path) :
    """sample id is just the basename without extension"""
    return os.path.basename(config_path).replace('.py','')

def __get_path__(relative_dir, sample_id = None) :
    if sample_id is None :
        sample_id = sample_id_from_path(sys.argv[0])
    os.makedirs(relative_dir, exist_ok=True)
    return f'{relative_dir}/{sample_id}.root'

def get_gold_path(sample_id = None) :
    return __get_path__('gold',sample_id)

def get_gold_label() :
    label = 'trunk'
    return label

def get_test_label() :
    br = 'local-test'
    if 'GITHUB_REF' in os.environ :
        br = os.environ['GITHUB_REF']
    br = br.replace('refs/tags/','').replace('refs/heads/','')
    return br

def get_test_path(sample_id = None) :
    return __get_path__('hist',sample_id)

def get_hist_path() :
    return get_test_path()

def get_events_path(sample_id = None) :
    return __get_path__('events',sample_id)

def list_configs(configs_dir = 'configs') :
    return [f'{configs_dir}/{c}' for c in os.listdir(configs_dir) if c.endswith('.py')]

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

def compare(sample_id) :
    gold = HistogramFile(get_gold_path(sample_id),get_gold_label(),ROOT.kRed ,True )
    test = HistogramFile(get_test_path(sample_id),get_test_label(),ROOT.kBlue,False)

    c = ROOT.TCanvas()
    c.SetLogy()

    out_dir = f'plots/{sample_id}'
    os.makedirs(out_dir,exist_ok=True)
    os.makedirs(f'{out_dir}/pass',exist_ok=True)
    os.makedirs(f'{out_dir}/fail',exist_ok=True)
    
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
        c.SaveAs(f'{out_dir}/{sub_dir}/{key.replace("/","_").replace(":","_")}.pdf')

if __name__ == '__main__' :
    import sys
    compare(sys.argv[1])
