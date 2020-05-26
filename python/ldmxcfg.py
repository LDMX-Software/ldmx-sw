#!/usr/bin/python

import LDMX.Framework.histogram as h

class Producer:
    def __init__(self, instanceName, className):
        
        self.instanceName=instanceName
        self.className=className
        self.parameters=dict()
        self.histograms=[]

    def build1DHistogram(self, name, xlabel, bins, xmin, xmax):
        self.histograms.append(h.histogram1D(name, xlabel, bins, xmin, xmax))
        return self

    def __str__(self) :
        msg = "\n  Producer(%s of class %s)"%(self.instanceName,self.className)
        if len(self.parameters)>0:
            msg += "\n   Parameters:"
            for k, v in self.parameters.items():
                msg += "\n    " + str(k) + " : " + str(v)

        if self.histograms:
            msg += "\n   Creating the following histograms:" 
            for histo in self.histograms: 
                msg += '\n    ' + str(histo)

        return msg

class Analyzer:
    def __init__(self, instanceName, className):
    
        self.instanceName=instanceName
        self.className=className
        self.parameters=dict()
        self.histograms=[]
   
    def build1DHistogram(self, name, xlabel, bins, xmin, xmax):
        self.histograms.append(h.histogram1D(name, xlabel, bins, xmin, xmax))
        return self

    def __str__(self) :
        msg = "\n  Analyzer(%s of class %s)"%(self.instanceName,self.className)
        if len(self.parameters)>0:
            msg += "\n   Parameters:"
            for k, v in self.parameters.items():
                msg += "\n    " + str(k) + " : " + str(v)

        if self.histograms:
            msg += "\n   Creating the following histograms:" 
            for histo in self.histograms: 
                msg += '\n    ' + str(histo)

        return msg
                
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
        self.logFrequency=-1
        self.compressionSetting=9
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

    def setCompression(self,algorithm,level=9):
        """set the compression settings for any output files in this process

        We combine the compression settings here in the same way that ROOT
        does. This allows the compression settings to be passed along as
        one integer rather than two without any loss of generality.

        Look at ROOT's documentation for TFile to learn more
        about the different compression algorithms and levels available
        (as well as what integers to use).

        Parameters
        ----------
        algorithm : int
            flag for the algorithm to use
        level : int
            flag for the level of compression to use
        """

        self.compressionSetting = algorithm*100 + level

    def __str__(self):
        msg = "Process with pass name '%s'"%(self.passName)
        if (self.run>0): msg += "\n using run number %d"%(self.run)
        if (self.maxEvents>0): msg += "\n Maximum events to process: %d"%(self.maxEvents)
        else: msg += "\n No limit on maximum events to process"
        msg += "\n Processor sequence:"
        for proc in self.sequence:
            msg += str(proc)
        if len(self.inputFiles) > 0:
            if len(self.outputFiles)==len(self.inputFiles):
                msg += "\n Files:"
                for i in range(0,len(self.inputFiles)):
                    msg += "\n  '%s' -> '%s'"%(self.inputFiles[i],self.outputFiles[i])
            else:
                msg += "\n Input files:"
                for afile in self.inputFiles:
                    msg += '\n  ' + afile
                if len(self.outputFiles) > 0:
                    msg += "\n Output file: " + self.outputFiles[0]
        elif len(self.outputFiles) > 0:
            msg += "\n Output file: " + self.outputFiles[0]
        msg += "\n Skim rules:"
        if self.skimDefaultIsKeep: msg += "\n  Default: keep the event"
        else: msg += "\n  Default: drop the event"
        for i in range(0,len(self.skimRules)-1,2):
            if self.skimRules[i+1]=="": 
                msg += "\n  Listen to hints from processors with names matching '%s'"%(self.skimRules[i])
            else:
                msg += "\n  Listen to hints with labels matching '%s' from processors with names matching '%s'"%(self.skimRules[i+1],self.skimRules[i])
        if len(self.keep) > 0:
            msg += "\n Rules for keeping previous products:"
            for arule in self.keep:
                msg += '\n  ' + arule
        if len(self.libraries) > 0:
            msg += "\n Shared libraries to load:"
            for afile in self.libraries:
                msg += '\n  ' + afile

        return msg

    
