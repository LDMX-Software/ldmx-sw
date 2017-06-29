#
# Sample configuration file that can be used to run the PN re-weighting 
# processor.
#

import sys

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").  Usually, this is set to recon.
p = ldmxcfg.Process("recon")

# Currently, we need to explicitly identify plugin libraries which should be
# loaded.  In future, we do not expect this be necessary
p.libraries.append("libEventProc.so")

# Load the PN re-weighting processor
pn_reweight = ldmxcfg.Producer("pn_reweight", "ldmx::PnWeightProcessor")

# Set the W threshold above which an event will be re-weighted.  For the 
# definition of W, see the processor.
pn_reweight.parameters["w_threshold"] = 1150.
pn_reweight.parameters["w_theta"] = 100.

# Define the sequence of event processors to be run
p.sequence = [pn_reweight]

# Provide the list of input files to run on
p.inputFiles=["ldmx_sim_events.root"]

# Provide the list of output files to produce, either one to contain the results of all input files or one output file name per input file name
p.outputFiles=["ldmx_digi_events.root"]

# Utility function to interpret and print out the configuration to the screen
p.printMe()
