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
    parser.add_argument('-o', nargs=1, help="string to append to file name")
    parser.add_argument('-t', nargs=1, help="recon python template file", required=True)
    parser.add_argument('-d', nargs=1, help="output directory", required=True)
    parser.add_argument('-x', action='store_true', help="enable dry run with no job submission", required=False)
    parser.add_argument('-s', action='store_true', help="skip job if output file already exists", required=False)
    args = parser.parse_args()

    input_pattern = args.i[0]
    if args.o is not None:
        output_append = args.o[0]
    else:
        output_append = "recon"
    output_dir = os.path.abspath(args.d[0])
    template_file = os.path.abspath(args.t[0])
    skip_existing = args.s
    dryrun = args.x

    input_glob = glob.glob('%s*.root' % (input_pattern))

    if len(input_glob) == 0:
        raise Exception("ERROR: No input files found matching '%s'." % (input_pattern))

    exe, err = subprocess.Popen(['which', RUN_SCRIPT], stdout=subprocess.PIPE).communicate()
    if 'no %s in' % RUN_SCRIPT in exe:
        raise Exception("ERROR: The %s script was not found!  (Is it in the path?)" % RUN_SCRIPT)
    exe = exe[:-1]

    submitted = 0
    for input_file in input_glob:                
        output_file = os.path.splitext(os.path.basename(input_file))[0] + "_" + output_append
        log_file = os.path.join(output_dir, output_file + ".log")
        input_path = os.path.abspath(input_file)
        if not (skip_existing and os.path.exists(os.path.join(output_dir, output_file))):
            if os.path.exists(log_file):
                subprocess.Popen('rm %s' % log_file, shell=True).wait()
            cmd = 'bsub -W %d:0 -q long -o %s -e %s python %s -o %s.root -i %s -t %s -d %s' % \
                    (12, log_file, log_file, exe, output_file, input_path, template_file, output_dir)
            print cmd
            if not dryrun:
                subprocess.Popen(cmd, shell=True).wait()
                submitted += 1
        else:
            print "Skipping submission for '%s' which already exists in '%s'." % (output_file, output_dir)
                
    if dryrun:
        print "\nWARNING: Dry run was enabled.  No jobs were submitted!"
    else:
        print "\nSubmitted %d LSF jobs." % submitted

if __name__ == '__main__' :
    main()
