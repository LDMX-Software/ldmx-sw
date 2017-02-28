#!/usr/bin/python

from LDMX.Framework import ldmxcfg;

p=ldmxcfg.Process("plot")

testplot=ldmxcfg.Analyzer("testplot", "ldmx::DummyAnalyzer")

testplot.parameters["caloHitCollection"]="hcalDigis"

p.sequence=[testplot]
p.inputFiles=["ldmx_digi_events.root"]
p.maxEvents=50
p.histogramFile="histo.root"

p.printMe()

