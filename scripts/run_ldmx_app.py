#!/usr/bin/env python

import argparse
import os
import getpass
import subprocess
from string import Template

def main():

    parser = argparse.ArgumentParser(description="Run an ldmx-sim job")
    parser.add_argument('-o', nargs=1, help="output ROOT file", required=True)
    parser.add_argument('-i', nargs=1, help="input ROOT file", required=True) 
    parser.add_argument('-t', nargs=1, help="python template file", required=True)
    parser.add_argument('-d', nargs=1, help="output directory", required=True)
    args = parser.parse_args()

    if not os.environ.get('LDMXSW_DIR'):
        raise Exception("ERROR: LDMXSW_DIR is not set in the environment!")
    ldmxsw = os.environ.get('LDMXSW_DIR')

    output_file = args.o[0]
    input_file = args.i[0]
    tmpl_file = os.path.abspath(args.t[0])
    output_dir = os.path.abspath(args.d[0])

    if os.path.exists(output_file):
        raise Exception("ERROR: The output file %s already exists!" % output_file)

    # create and cd to local tmp dir
    tmp_dir = mk_tmpdir()
    os.chdir(tmp_dir)

    # HACK: Symlink data items from Configuration/data e.g. for running the ecal veto BDT.
    config_dir = ldmxsw + '/Configuration/data'
    for item in os.listdir(config_dir):
        src = config_dir+'/'+item
        targ = tmp_dir+'/'+item
        os.symlink(src, targ)

    # substitute input and output files into the config template
    val_dict = {'input_file':input_file, 'output_file':output_file}
    config_file = create_config_file(tmpl_file, val_dict)

    command = 'ldmx-app %s' % config_file
    subprocess.Popen(command, shell=True).wait()

    cp_cmd = 'cp -r %s/%s %s' % (tmp_dir, output_file, output_dir)
    os.system(cp_cmd)

    # delete scratch dir
    subprocess.Popen('rm -rf %s' % tmp_dir, shell=True).wait()

def mk_tmpdir():
    scratch_dir = '/scratch/'+getpass.getuser()
    if not os.path.exists(scratch_dir):
        os.makedirs(scratch_dir)
    tmp_dir = '%s/%s' % (scratch_dir, os.environ['LSB_JOBID'])
    if not os.path.exists(tmp_dir):
        os.makedirs(tmp_dir)
    return tmp_dir

def create_config_file(tmpl_file, val_dict):

    tf = open(tmpl_file)
    src = Template(tf.read())
    res = src.substitute(val_dict)
    print "Created config from template '%s'" % tmpl_file
    print
    print res
    print
    of = open(os.path.basename(tmpl_file.replace('.tmpl', '')), 'w')
    of.write(res)
    of.close()
    return of.name

if __name__ == '__main__' :
    main()
