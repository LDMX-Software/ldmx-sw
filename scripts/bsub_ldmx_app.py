#!/usr/bin/env python

import os
import argparse
import math
import subprocess
import glob

RUN_SCRIPT = 'run_ldmx_app.py'

def main():

    parser = argparse.ArgumentParser(description="Run an ldmx-app job on LSF")
    parser.add_argument('-i', nargs=1, help="input string for file glob", required=True)
    parser.add_argument('-o', nargs=1, help="output file base name", required=True)
    parser.add_argument('-t', nargs=1, help="recon python template file", required=True)
    parser.add_argument('-d', nargs=1, help="base dir for input files and output directory", required=True)
    parser.add_argument('-x', action='store_true', help="enable dry run with no job submission", required=False)
    args = parser.parse_args()

    input_pattern = args.i[0]
    input_dir = os.path.abspath(args.d[0])
    output_basename = args.o[0]
    output_dir = os.path.abspath(args.d[0])
    template_file = os.path.abspath(args.t[0])
    dryrun = args.x

    input_glob = glob.glob('%s/%s*.root' % (input_dir, input_pattern))

    if len(input_glob) == 0:
        raise Exception("ERROR: No input files found matching '%s' in dir '%s'." % (input_pattern, input_dir))

    exe, err = subprocess.Popen(['which', RUN_SCRIPT], stdout=subprocess.PIPE).communicate()
    if 'no %s in' % RUN_SCRIPT in exe:
        raise Exception("ERROR: The %s script was not found!  (Is it in the path?)" % RUN_SCRIPT)
    exe = exe[:-1]

    for input_file in input_glob:

        fnum = input_file[input_file.rfind('_')+1:input_file.rfind('.')]
        output_file = '%s_%s' % (output_basename, fnum)
        log_file = '%s/%s.log' % (output_dir, output_file)
        if os.path.exists(log_file):
            subprocess.Popen('rm %s' % log_file, shell=True).wait()
        cmd = 'bsub -W %d:0 -q long -o %s -e %s python %s -o %s.root -i %s -t %s -d %s' % \
                (12, log_file, log_file, exe, output_file, input_file, template_file, output_dir)
        print cmd

        if not dryrun:
            subprocess.Popen(cmd, shell=True).wait()

    if dryrun:
        print "WARNING: Dry run was enabled.  No jobs were submitted!"

if __name__ == '__main__' :
    main()
