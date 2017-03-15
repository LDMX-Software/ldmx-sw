#!/usr/bin/env python

import os
import argparse
import math
import subprocess

DEFAULT_DETECTOR = "ldmx-det-full-v2-fieldmap"
RUN_SCRIPT = "run_ldmx_sim.py"

def main():

    parser = argparse.ArgumentParser(description="Submit ldmx-sim LSF jobs using a particle gun macro")
    parser.add_argument('-m', nargs="+", help="macros", required=True)         
    parser.add_argument('-d', nargs=1, help="detector name")                   
    parser.add_argument('-n', nargs=1, help="number of events", required=True) 
    parser.add_argument('-p', nargs=1, help="output directory", required=True) 
    parser.add_argument('-o', nargs=1, help="output file base name", required=True)
    parser.add_argument('-j', nargs=1, help="number of jobs", required=True)
    parser.add_argument('-x', action='store_true', help="enable dry run with no job submission")
    args = parser.parse_args()

    macros = args.m
    if args.d is not None:
        detector = args.d[0]
    else:
        detector = DEFAULT_DETECTOR
    nevents = int(args.n[0])
    outputdir = os.path.abspath(args.p[0])
    filename = args.o[0]
    jobs = int(args.j[0])
    dryrun = args.x

    jobtime = 2*round(float(nevents)/60./ 60., 0)
    if jobtime == 0:
        jobtime = 1

    exe, err = subprocess.Popen(['which', RUN_SCRIPT], stdout=subprocess.PIPE).communicate()
    if "no %s in" % RUN_SCRIPT in exe:
        raise Exception("ERROR: The %s script was not found!  (Is it in the path?)" % RUN_SCRIPT)
    exe = exe[:-1]

    for jobnum in xrange(1, jobs + 1):
        output_file = "%s_%04d" % (filename, jobnum)
        log_file = "%s/%s.log" % (outputdir, output_file)
        if os.path.exists(log_file):
            subprocess.Popen("rm %s" % log_file, shell=True).wait()
        macrostr = ' '.join([os.path.abspath(m) for m in macros]) 
        cmd = "bsub -W %d:0 -q long -o %s -e %s python %s -o %s.root -m %s -d %s -n %d -p %s" % \
                (jobtime, log_file, log_file, exe, output_file, macrostr, detector, nevents, outputdir)
        print cmd
        if not dryrun:
            subprocess.Popen(cmd, shell=True).wait()

    if dryrun:
        print "\nWARNING: Dry run was enabled.  No jobs were submitted!"

if __name__ == "__main__" :
    main()
