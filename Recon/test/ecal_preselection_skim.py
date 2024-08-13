import os import sys

    thisPassName = "presel" inputName = sys.argv[1]

                                                 from LDMX.Framework import ldmxcfg p = ldmxcfg.Process(thisPassName)

                                                                                                            p.termLogLevel = 0

                                                                                                        p.inputFiles =[inputName] p.histogramFile = "histosPreskimmed.root" p.outputFiles =["eventsPreskimmed.root"]

                                                                                                                                                                                            from LDMX.Recon.ecalPreselectionSkimmer import ecalPreselectionSkimmer ecalPreselectionSkimmer.summed_tight_iso_max = 1100. ecalPreselectionSkimmer.n_readout_hits_max = 90
''' ## #Reminder for the possible things to cut on

                                                                                                                                                                                            ecalPreselectionSkimmer.summed_det_max = 9999. ecalPreselectionSkimmer.summed_tight_iso_max = 9999. ecalPreselectionSkimmer.ecal_back_energy_max = 9999. ecalPreselectionSkimmer.n_readout_hits_max = 9999 ecalPreselectionSkimmer.shower_rms_max = 9999 ecalPreselectionSkimmer.shower_y_std_max = 9999 ecalPreselectionSkimmer.shower_x_std_max = 9999 ecalPreselectionSkimmer.max_cell_dep_max = 9999. ecalPreselectionSkimmer.std_layer_hit_max = 9999 ecalPreselectionSkimmer.n_straight_tracks_max = 9999
'''

                                                                                                                                                                                            p.sequence =[ecalPreselectionSkimmer] p.skimDefaultIsDrop() p.skimConsider(p.sequence[0].instanceName)
