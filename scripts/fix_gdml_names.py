#!/usr/bin/env python

"""

  Fix GDML attributes written by the G4GDMLParser class.

    * strip pointer strings from name attributes
    * strip pointer strings from ref attributs
    * append copynumber to physvol names to make them unique
    * replace position and rotation names with one based on corrected physvol name

  Writes new GDML file, replacing the one given from the command line argument.

  author: Jeremy McCormick, SLAC

"""

import sys
import xml.etree.ElementTree

if sys.argc == 1:
    raise Exception("Missing name of GDML file")

fname = sys.argv[1]

et = xml.etree.ElementTree.parse(fname)
root = et.getroot()

debug = True

def strip_pointer_string(name):
    try:
        pointer_start_index = name.index("0x")
        pointer_end_index = pointer_start_index + 9
        pointer_string = name[pointer_start_index:pointer_end_index]
        return name.replace(pointer_string, '')
    except:
        return name

for child in root.iter():
    if 'name' in child.attrib:

        if debug:
            print "old name: " + child.attrib['name']

        # strip pointer strings from name attributes
        child.attrib['name'] = strip_pointer_string(child.attrib['name'])

        # append string to make PV names unique
        if child.tag == "physvol":
            child.attrib['name'] = child.attrib['name'] + "_physvol" 
            if 'copynumber' in child.attrib:
                child.attrib['name'] = child.attrib['name'] + child.attrib['copynumber']
            for pv_child in child:
                if pv_child.tag == "rotation":
                    if debug:
                        print "old rot name: " + pv_child.attrib['name']
                    pv_child.attrib['name'] = child.attrib['name'] + "_rot"
                    if debug:
                        print "new rot name: " + pv_child.attrib['name']
                if pv_child.tag == "position":
                    if debug:
                        print "old pos name: " + pv_child.attrib['name']
                    pv_child.attrib['name'] = child.attrib['name'] + "_pos"
                    if debug:
                        print "new pos name: " + pv_child.attrib['name']

        if debug:
            print "new name: " + child.attrib['name']
    
    if 'ref' in child.attrib:

        if debug:
            print "old ref: " + child.attrib['ref']

        # strip pointer strings from ref attributes
        child.attrib['ref'] = strip_pointer_string(child.attrib['ref'])

        if debug:
            print "new ref: " + child.attrib['ref']

et.write(fname) 
