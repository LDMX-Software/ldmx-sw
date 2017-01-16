#!/usr/bin/python

class Producer:
    def __init__(self, instanceName, className):
        self.instanceName=instanceName
        self.className=className
        self.parameters=dict()
    def printMe(self):
        printMe(self,"")
    def printMe(self,prex):
        print "%sProducer(%s of class %s)"%(prex,self.instanceName,self.className)
        if len(self.parameters)>0:
            print "%s Parameters:"%(prex)
            for k, v in self.parameters.items():
                print prex,"  ",k," : ",v

class Analyzer:
    def __init__(self, instanceName, className):
        self.instanceName=instanceName
        self.className=className
        self.parameters=dict()
    def printMe(self):
        printMe(self,"")
    def printMe(self,prex):
        print "%sAnalyzer(%s of class %s)"%(prex,self.instanceName,self.className)        
        if len(self.parameters)>0:
            print "%s Parameters:"%(prex)
            for k, v in self.parameters.items():
                print prex,"  ",k," : ",v
                
class Process:
    lastProcess=None
    
    def __init__(self, passName):
        self.passName=passName
        self.maxEvents=-1
        self.run=-1
        self.inputFiles=[]
        self.sequence=[]
        self.keep=[]
        self.libraries=[]
        Process.lastProcess=self

    def printMe(self):
        print "Process with pass name '%s'"%(self.passName)
        if (self.run>0): print " using run number %d"%(self.run)
        if (self.maxEvents>0): print " Maximum events to process: %d"%(self.maxEvents)
        else: " No limit on maximum events to process"
        print "Processor sequence:"
        for proc in self.sequence:
            proc.printMe("  ")
        if len(self.inputFiles) > 0:
            print "Input files:"
            for afile in self.inputFiles:
                print "   %s"%(afile)
        if len(self.keep) > 0:
            print "Rules for keeping previous products:"
            for arule in self.keep:
                print "   %s"%(afile)
        if len(self.libraries) > 0:
            print "Shared libraries to load:"
            for afile in self.libraries:
                print "   %s"%(afile)

    
