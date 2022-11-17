plots = []
histograms = []

class Test_plots :
    
    def dqm() :
        plots = [('TrigScintDigiPad1/TrigScintDigiPad1_n_hits', 'Hit multiplicity'),
                    ('TrigScintTracks/TrigScintTracks_n_tracks', 'Track multiplicity'),
#                    ('EcalShowerFeatures/EcalShowerFeatures_num_readout_hits', 'N Readout Hits')
                    ]

        return plots


    def branchPlots(passName="valid", treeName="LDMX_Events") :


        collections=["Sim", "Digi", "Cluster"]
        pads = ["Pad1", "Pad2", "Pad3"] 
        tColl="TrigScintTracks"


        histograms = [(f'{treeName}/EcalSimHits_{passName}/EcalSimHits_{passName}.edep_',
              '"Sim Energy Dep [MeV]"', 50, 0,30, 'edep')
        ]

        for pad in pads :
#            for coll in collections :
            histograms += [(f'{treeName}/Trigger{pad}Clusters_{passName}.centroid_',
                            '"Cluster centroid [barID]"', 60, 0,60, f'clusterCentroid_{pad}')
            ]
        
        return histograms
