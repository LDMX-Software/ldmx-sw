"""Import EventModel dictionary into PyROOT

This needs us to import ROOT and then we can load
the Event library which also pulls in the Event dictionary.

Examples
--------
Use the following line in your python script to inform PyROOT of our Event Model:
    from LDMX.Recon import dictionary
"""

import ROOT
ROOT.gSystem.Load( '@CMAKE_INSTALL_PREFIX@/lib/libRecon.so' )
