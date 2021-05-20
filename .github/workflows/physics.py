
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

def get_test_path(sample_id = None) :
    return __get_path__('test',sample_id)

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

def compare(sample_id, gold_label = 'gold', test_label = 'test') :
    gold = HistogramFile(get_gold_path(sample_id),gold_label,ROOT.kRed ,True )
    test = HistogramFile(get_test_path(sample_id),test_label,ROOT.kBlue,False)

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
        c.SaveAs(f'{out_dir}/{sub_dir}/{key.replace("/","_")}.pdf')

def generate_events(config) :
    get_test_path()
    get_events_path()
    os.system(f'fire {config}')
    return sample_id_from_path(config)

def generate_golden(arg) :
    for f in list_configs() :
        sample_id = generate_events(f)
        old_test = get_test_path(sample_id)
        new_gold = get_gold_path(sample_id)
        os.system(f'mv {old_test} {new_gold}')

def validation(arg) :
    sample_id = generate_events(arg.config)
    compare(sample_id,arg.gold_label,arg.test_label)

if __name__ == '__main__' :
    import argparse, sys
        # Parse
    parser = argparse.ArgumentParser(f'ldmx python3 {sys.argv[0]}',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    subparsers = parser.add_subparsers(help='Choose which action to perform.')

    # Generation
    parse_gen = subparsers.add_parser('gen', help='Generate golden histograms using current compilation of ldmx-sw.')
    parse_gen.set_defaults(action=generate_golden)

    # Validation
    parse_val = subparsers.add_parser('val', help='Validate current compilation of ldmx-sw.')
    parse_val.add_argument('config',help='Config script to run for validation.')
    parse_val.add_argument('test_label',help='Label to use for current compilation of ldmx-sw.')
    parse_val.add_argument('gold_label',help='Label to use for golden histograms.')
    parse_val.set_defaults(action=validation)

    arg = parser.parse_args()

    if 'action' not in arg :
        parser.error('Must choose an action to perform.')

    arg.action(arg)
