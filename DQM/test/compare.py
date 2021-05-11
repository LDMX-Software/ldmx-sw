
import os
import ROOT
ROOT.gSystem.Load('libFramework.so')
ROOT.gROOT.SetBatch(1)
ROOT.gStyle.SetOptStat(0)

def translate_name(image) :
    return image.replace('/','_').replace(':','_')

def get_hist(tree, branch, image, color, fill) :
    tree_name = translate_name(image)
    tree.Draw(f'{branch}>>h_{tree_name}_{branch}')
    h = ROOT.gDirectory.Get(f'h_{tree_name}_{branch}')
    h.SetTitle(f'{image};{branch}')
    h.SetLineColor(color)
    h.SetLineWidth(2)
    h.SetMarkerColor(color)
    if fill :
        h.SetFillColor(color)
    return h

class ParallelTrees() :
    def __init__(self, trunk_image, dev_image, sample_id) :
        self.__trunk_image = trunk_image
        self.__dev_image = dev_image
        self.__sample_id = sample_id

        self.__trunk_color = ROOT.kRed
        self.__trunk_fill  = True
        self.__dev_color = ROOT.kBlue
        self.__dev_fill = False

        self.__trunk_file = ROOT.TFile.Open(f'{translate_name(trunk_image)}_{sample_id}.root')
        self.__trunk_tree = self.__trunk_file.Get('LDMX_Events')

        def retrieve_branches(l) :
            flat_l = []
            for b in l :
                if b.GetListOfBranches().GetEntries() > 0 :
                    flat_l.extend(retrieve_branches(b.GetListOfBranches()))
                else :
                    flat_l.append(b.GetFullName())

            return flat_l

        # Determine lowest-level branches
        self.__branches = retrieve_branches(self.__trunk_tree.GetListOfBranches())
        print(self.__branches)

        self.__dev_file = ROOT.TFile.Open(f'{translate_name(dev_image)}_{sample_id}.root')
        self.__dev_tree = self.__dev_file.Get('LDMX_Events')

    def get_hist(self, branch) :
        h_trunk = get_hist(self.__trunk_tree, branch, self.__trunk_image, self.__trunk_color, self.__trunk_fill)
        h_dev = get_hist(self.__dev_tree, branch, self.__dev_image, self.__dev_color, self.__dev_fill)

        return h_trunk, h_dev

    def comp(self) :

        c = ROOT.TCanvas()
        c.SetLogy()
        os.system(f'mkdir -p {self.__sample_id}')
        
        for br in self.__branches :
            h_trunk, h_dev = self.get_hist(br)
        
            if 'TH' not in h_trunk.__class__.__name__ :
                continue

            if 'TH' not in h_dev.__class__.__name__ :
                continue

            h_trunk.Draw()
            h_dev.Draw('same')
        
            c.BuildLegend()
            c.SaveAs(f'{self.__sample_id}/{br}.pdf')


if __name__ == '__main__' :
    import sys
    pt = ParallelTrees(sys.argv[1], sys.argv[2], sys.argv[3])
    pt.comp()
