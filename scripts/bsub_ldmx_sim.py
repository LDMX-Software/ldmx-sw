#!/usr/bin/env python

import os
import argparse
import math
import subprocess
import sys

DEFAULT_DETECTOR = 'ldmx-det-full-v2-fieldmap'
RUN_SCRIPT = 'run_ldmx_sim.py'

def main():

    parser = argparse.ArgumentParser(description='Submit ldmx-sim LSF jobs')
    parser.add_argument('-m', action='append', dest='m', default=[], help="macros to run", required=False) 
    parser.add_argument('-d', nargs=1, help="detector name", required=False) 
    parser.add_argument('-n', nargs=1, help="number of events", required=True) 
    parser.add_argument('-p', nargs=1, help="output directory", required=True) 
    parser.add_argument('-o', nargs=1, help="output file base name", required=True)
    parser.add_argument('-i', action='append', dest='i', default=[], help="input files", required=False)
    parser.add_argument('-j', nargs=1, help="number of jobs (cannot be used with -i)", required=False)
    parser.add_argument('-x', action='store_true', help="enable dry run with no job submission", required=False)
    parser.add_argument('-s', action='store_true', help="skip job if output file already exists", required=False)
    args = parser.parse_args()

    macros = args.m
    if args.d is not None:
        detector = args.d[0]
    else:
        detector = DEFAULT_DETECTOR
    nevents = int(args.n[0])
    outputdir = os.path.abspath(args.p[0])
    filename = args.o[0]
    dryrun = args.x
    skip_existing = args.s
    input_files = args.i

    if args.j is not None:
        if len(input_files):
            raise Exception("ERROR: The -i and -j arguments cannot be used together.")
        jobs = int(args.j[0])
    else:
        jobs = len(input_files)

    jobtime = 2*round(float(nevents)/60./ 60., 0)
    if jobtime == 0:
        jobtime = 1

    exe, err = subprocess.Popen(['which', RUN_SCRIPT], stdout=subprocess.PIPE).communicate()
    if 'no %s in' % RUN_SCRIPT in exe:
        raise Exception("ERROR: The %s script was not found!  (Is it in the path?)" % RUN_SCRIPT)
    exe = exe[:-1]

    macrostr = ' '.join([os.path.abspath(m) for m in macros])

    for jobnum in xrange(1, jobs + 1):
        output_file = '%s_%04d' % (filename, jobnum)
        log_file = '%s/%s.log' % (outputdir, output_file)
        if not (skip_existing and os.path.exists(os.path.join(output_dir, output_file + ".root"))):
            if os.path.exists(log_file):
                subprocess.Popen('rm %s' % log_file, shell=True).wait()                
            cmd = 'bsub -W %d:0 -q long -o %s -e %s python %s -o %s.root -d %s -n %d -p %s' % \
                (jobtime, log_file, log_file, exe, output_file, detector, nevents, outputdir)
            if len(macrostr):
                cmd = '%s -m %s' % (cmd, macrostr)
            if len(input_files):
                input_file = os.path.abspath(input_files[jobnum-1])
                cmd = '%s -i %s' % (cmd, input_file)
            print cmd
            if not dryrun:
                subprocess.Popen(cmd, shell=True).wait()
        else:
            print "Skipping submission for '%s' which already exists in '%s'." % (output_file, output_dir)

    if dryrun:
        print "\nWARNING: Dry run was enabled.  No jobs were submitted!"
    else:
        print "\nSubmitted %d LSF jobs." % jobs

if __name__ == '__main__' :
    main()
