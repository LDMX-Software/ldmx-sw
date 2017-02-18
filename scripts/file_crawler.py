#!/usr/bin/python

import argparse
import sys
import ROOT as r

def main() : 
   
    # Parse all command line arguments using the argparse module
    parser = argparse.ArgumentParser(description='')
    parser.add_argument("-l", "--file_list", 
                        help="List of files to crawl.")
    parser.add_argument("-t", "--tree_name", 
                        help="Name of the ROOT tree to crawl.")
    args = parser.parse_args()

    if not args.file_list: 
        print 'Please specify a list of files to process.'
        sys.exit(2)
   
    f = open(args.file_list, 'r')

    total_entries = 0
    for root_file_path in f: 
        print 'Crawling file ' + root_file_path.rstrip()
        root_file = r.TFile(root_file_path.rstrip())
        tree = root_file.Get(args.tree_name)
        total_entries += tree.GetEntries()
        root_file.Close()
        
    print "Total number of entries %s" % total_entries

if __name__ == "__main__":
    main()
