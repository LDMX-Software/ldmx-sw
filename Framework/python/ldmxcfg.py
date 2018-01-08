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
        self.outputFiles=[]
        self.sequence=[]
        self.keep=[]
        self.libraries=[]
        self.skimDefaultIsKeep=True
        self.skimRules=[]
        Process.lastProcess=self

    def skimDefaultIsSave(self):
        self.skimDefaultIsKeep=True
        
    def skimDefaultIsDrop(self):
        self.skimDefaultIsKeep=False

    def skimConsider(self,namePat):
        self.skimRules.append(namePat)
        self.skimRules.append("")

    def skimConsiderLabelled(self,namePat,labelPat):
        self.skimRules.append(namePat)
        self.skimRules.append(labelPat)

    def printMe(self):
        print "Process with pass name '%s'"%(self.passName)
        if (self.run>0): print " using run number %d"%(self.run)
        if (self.maxEvents>0): print " Maximum events to process: %d"%(self.maxEvents)
        else: " No limit on maximum events to process"
        print "Processor sequence:"
        for proc in self.sequence:
            proc.printMe("  ")
        if len(self.inputFiles) > 0:
            if len(self.outputFiles)==len(self.inputFiles):
                print "Files:"
                for i in range(0,len(self.inputFiles)):
                    print "   '%s' -> '%s'"%(self.inputFiles[i],self.outputFiles[i])
            else:
                print "Input files:"
                for afile in self.inputFiles:
                    print "   %s"%(afile)
                if len(self.outputFiles) > 0:
                    print "Output file:", self.outputFiles[0]
        elif len(self.outputFiles) > 0:
            print "Output file:", self.outputFiles[0]
        print "Skim rules:"
        if self.skimDefaultIsKeep: print " Default: keep the event"
        else: print " Default: drop the event"
        for i in range(0,len(self.skimRules)-1,2):
            if self.skimRules[i+1]=="": 
                print " Listen to hints from processors with names matching '%s'"%(self.skimRules[i])
            else:
                print " Listen to hints with labels matching '%s' from processors with names matching '%s'"%(self.skimRules[i+1],self.skimRules[i])
        if len(self.keep) > 0:
            print "Rules for keeping previous products:"
            for arule in self.keep:
                print "   %s"%(arule)
        if len(self.libraries) > 0:
            print "Shared libraries to load:"
            for afile in self.libraries:
                print "   %s"%(afile)

    
