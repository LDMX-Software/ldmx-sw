"""Fully-runnable example that uses the biased ecal pn template"""

from LDMX.Framework import ldmxcfg
from LDMX.Biasing import ecal
from LDMX.SimApplication import generators

# We need to create a process
#   this is the object that keeps track of all the files/processors/histograms/etc
#   the input name is a shortcode to distinguish this run
p=ldmxcfg.Process("ecalpn")

# We need to tell the process what libraries are required
#   Here we are using a biased simulation, so we need both of those libraries
p.libraries.append("libSimApplication.so")
p.libraries.append("libBiasing.so")

# We import the Ecal PN template, the two arguments allow you to specify the geometry
#   and the generator you wish to use
ecal_pn = ecal.photo_nuclear('ldmx-det-v12', generators.single_4gev_e_upstream_tagger())

# Put the simulation into the process sequence
p.sequence=[ecal_pn]

# Give a name for the output file
p.outputFiles=['ecal_pn.root']

# How many events should the process simulation?
p.maxEvents = 1000

# How frequently should the process print an update?
p.logFrequency = 100
