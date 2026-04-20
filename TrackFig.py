#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Apr 13 12:17:11 2026

@author: kvarn
"""

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('tracking')
p.input_files = ["TruthSamples.root"]
p.output_files = ["Tracks.root"]

#from LDMX.TrigScint.trig_scint import TrigScintTrackProducer

prod_tracks = ldmxcfg.Producer("lut_track", "trigscint::TrigScintTrackProducer",
                          "TrigScint")

LUT_tracks = ldmxcfg.Producer("prod_track", "trigscint::TrigScintLUTTracker", 
                              "TrigScint")

prod_tracks.seeding_collection = "TriggerPad1Clusters"
prod_tracks.further_input_collections = ["TriggerPad2Clusters", "TriggerPad3Clusters"]  
prod_tracks.delta_max = 1.0
prod_tracks.input_pass_name = ""
prod_tracks.verbosity = 1
prod_tracks.vertical_bar_start_index = 100
prod_tracks.number_horizontal_bars = 16
prod_tracks.horizontal_bar_width = 10.0
prod_tracks.horizontal_bar_gap = 1.0
prod_tracks.number_vertical_bars = 16
prod_tracks.vertical_bar_width = 10.0
prod_tracks.vertical_bar_gap = 1.0
prod_tracks.allow_skip_last_collection = False 

p.logger.term_level = 1

LUT_tracks.seeding_collection = "TriggerPad1Clusters"
LUT_tracks.further_input_collections = ["TriggerPad2Clusters", "TriggerPad3Clusters"]  
LUT_tracks.delta_max = 1.0
LUT_tracks.input_pass_name = ""
LUT_tracks.verbosity = 1
LUT_tracks.vertical_bar_start_index = 100
LUT_tracks.number_horizontal_bars = 16
LUT_tracks.horizontal_bar_width = 10.0
LUT_tracks.horizontal_bar_gap = 1.0
LUT_tracks.number_vertical_bars = 16
LUT_tracks.vertical_bar_width = 10.0
LUT_tracks.vertical_bar_gap = 1.0
LUT_tracks.allow_skip_last_collection = False 
LUT_tracks.lut_file = "LUT.txt"


prod_tracks.output_collection = "TrigScintTrackProdTracks"
LUT_tracks.output_collection = "TrigScintLUTTracks"


p.sequence = [#ldmxcfg.Analyzer.from_file('LUTAnalyzer.cxx', 
              #needs = ['TrigScint_Event', 'SimCore_Event']),
              prod_tracks, 
              #LUT_tracks
              ]