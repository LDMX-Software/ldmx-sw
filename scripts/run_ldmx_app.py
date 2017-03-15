import argparse
import os
import getpass
import subprocess
from string import Template

def main():

    parser = argparse.ArgumentParser(description='Run an ldmx-sim job')
    parser.add_argument('-o', nargs=1,  help="output ROOT file",     required=True)
    parser.add_argument('-i', nargs=1,  help="input ROOT file",      required=True) 
    parser.add_argument('-t', nargs=1,  help="python template file", required=True)
    parser.add_argument('-d', nargs=1,  help="output directory",     required=True)
    args = parser.parse_args()

    output_file = args.o[0]
    input_file = args.i[0]
    tmpl_file = os.path.abspath(args.t[0])
    output_dir = os.path.abspath(args.d[0])

    if os.path.exists(output_file):
        raise Exception("ERROR: The output file %s already exists!" % output_file)

    # create and cd to local tmp dir
    tmp_dir = mk_tmpdir('/scratch')
    os.chdir(tmp_dir)

    # substitute input and output files into the config template
    val_dict = {'input_file':input_file, 'output_file':output_file}
    config_file = create_config_file(tmpl_file, val_dict)

    print 'Created config file %s' % config_file

    command = "ldmx-app %s" % config_file
    print "Running '%s'" % command
    subprocess.Popen(command, shell=True).wait()

    cp_cmd = 'cp -r %s/%s %s' % (tmp_dir, output_file, output_dir)
    print cp_cmd
    os.system(cp_cmd)

    #rm_cmd = "rm -rf %s" % tmp_dir
    #print rm_cmd
    #subprocess.Popen(rm_cmd, shell=True).wait()

def mk_tmpdir(bdir):

    scratch_dir = bdir+'/'+getpass.getuser()

    if not os.path.exists(scratch_dir):
        os.makedirs(scratch_dir)

    tmp_dir = '%s/%s' % (scratch_dir, os.environ['LSB_JOBID'])
    tmp_dir = scratch_dir
    print 'Creating tmp directory %s' % tmp_dir
    if not os.path.exists(tmp_dir):
        os.makedirs(tmp_dir)

    return tmp_dir

def create_config_file(tmpl_file, val_dict):

    tf = open(tmpl_file)
    src = Template(tf.read())
    res = src.substitute(val_dict)
    print "Created config template from '%s'" % tmpl_file
    print res
    print
    of = open(tmpl_file.replace('.tmpl', ''), 'w')
    of.write(res)
    of.close()
    return of.name

if __name__ == "__main__" :
    main()
