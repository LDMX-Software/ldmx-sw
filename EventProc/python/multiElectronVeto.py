#!/usr/bin/python

from LDMX.Framework import ldmxcfg

multiEleVeto = ldmxcfg.Producer("multiEleVeto","ldmx::MultiElectronVeto")

multiEleVeto.parameters['verbose']=False

