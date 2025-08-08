#!/bin/bash

###############################################################################
# init.sh
#   Pre-validation initializing for IT Pileup Test Sample
#
#   We need to generate the main and pileup events for the OverlayProducer
#   to use in config.py.
###############################################################################

start_group Generate Main Events
denv fire gen_main.py
end_group

start_group Generate Pileup Events
denv fire gen_pileup.py
end_group
