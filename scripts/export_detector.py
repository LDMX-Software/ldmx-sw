#!/usr/bin/env python

import sys
from subprocess import Popen, PIPE

if len(sys.argv) < 3:
    print "Usage: export_detector.py [detector.gdml] [output.gdml]"
    sys.exit(1)

det_in = sys.argv[1]
det_out = sys.argv[2]
                
p = Popen(['ldmx-sim'], stdin=PIPE)
p.stdin.write("/ldmx/gdml/removeModule magnet.gdml\n")
p.stdin.write("/ldmx/gdml/read %s\n" % det_in)
p.stdin.write("/run/initialize\n")
p.stdin.write("/ldmx/gdml/write %s\n" % det_out)
p.stdin.write("exit\n")
p.stdin.close()
p.wait()
