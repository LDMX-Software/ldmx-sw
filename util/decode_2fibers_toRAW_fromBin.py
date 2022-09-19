import ROOT as r
#from sys import argv
import sys 
import os 
from optparse import OptionParser


class qie_frame:

    def __init__(self,label):
        self.label=label
        self.adcs=[]
        self.tdcs=[]
        self.peds=[]
        self.capid=-1
        self.ce=-1
        self.bc0=-1

    def decode(self,frame):
        if len(frame) != 22 :
            print("[[qie_frame]]  ERROR: was expecting 22 characters; got ", len(frame))
            return -1 #use this to skip this time sample later
        else :
            self.capid = (int(frame[:2],16)>>2)&3
            self.ce = (int(frame[:2],16)>>1)&1
            self.bc0 = (int(frame[:2],16))&1
            for i in range(8):
                temp_adc=int(frame[(i+1)*2:(i+2)*2],16)
                #print('adc{0}: {1}'.format(i,temp_adc))
                self.adcs.append(temp_adc)
            tdc_bytes_03=int(frame[-4:-2],16)
            tdc_bytes_48=int(frame[-2:],16)
            for i in range(3,-1,-1):
                temp_tdc=(tdc_bytes_03>>(i*2))&3
                self.tdcs.append(temp_tdc)
            for i in range(3,-1,-1):
                temp_tdc=(tdc_bytes_48>>(i*2))&3
                self.tdcs.append(temp_tdc)
            return 0 #if all is well 
                
    def print(self):
        print('-----QIE FRAME-------')
        print(self.label)
        print('CAPID: {0} CE: {1} BC0: {2}'.format(self.capid,self.ce,self.bc0))
        print('adcs: ',' '.join(map(str,self.adcs)))
        print('tdcs: ',' '.join(map(str,self.tdcs)))

    def triggerPrint(self):
        printFrame=False
        for adc in self.adcs:
            if adc > 100 and adc < 255:
                printFrame=True
                break
        for tdc in self.tdcs:
            if tdc != 3 :
                printFrame=True
                break
        if (printFrame) :
            print('[[qie_frame]] Trigger')
            self.print()
        return

    def triggerForReadout(self, doTDCtrig) :
        if doTDCtrig : # trigger on good tdc somewhere in the event
            for tdc in self.tdcs :
                if tdc < 3 :
                    print("triggering with tdc = "+str(tdc))
                    return True
        else :
            margin = 10 #adc counts 
            for adc,ped in zip(self.adcs, self.peds):
                if adc > 110 and adc < 255 :
                     print("Got ADC above 3000 fC: "+str(adc))
                if adc > ped+margin and adc < 255 :
                    print("triggering with adc = "+str(adc)+" > "+str(ped+margin))
                    return True
        return False

adcmax=150
tsmax=64
hist_pulse=[]
for i in range(16):
    hist_pulse.append(r.TH2F("pulse{0}".format(i),"pulse{0};time sample;ADC".format(i),tsmax,-0.5,tsmax-0.5,adcmax,-0.5,adcmax-0.5))

hist_adc=[]
for i in range(16):
    hist_adc.append(r.TH1F("adc{0}".format(i),"adc{0};ADC;Arbitrary".format(i),adcmax,-0.5,adcmax-0.5))
    
hist_tdc=[]
for i in range(16):
    hist_tdc.append(r.TH1F("tdc{0}".format(i),"tdc{0};TDC;Arbitrary".format(i),4,-0.5,3.5))

    
def clean_kchar(data):
    return data.replace('F7FB','')

def clean_BC7C(data):
    return data.replace('BC7C','')

def remove_partial_events(data):
    return  data[data.find('BC'):data.rfind('BC')]

def read_string_from_file(filename):
    data=""
    with open(filename) as f:
        data+= ''.join(f.readlines())
    return data.split('192.168.1.30')


def binToTxt(inFileName, outFileName):
    print('Processing file ',inFileName)
    with open(outFileName,'w') as outF:
 #       sys.stdout = outF
        with open(inFileName,'rb') as inF:
            data = inF.read(8)
            while data:
                hexline='\n'
                for dd in data:  #convert byte per byte
                    #format in hex with padding (don't lose leading zeroes)
                    hexnum="{0:0>2X}".format(dd, 'x') #int(hexnum,2))
                    #print(hexnum)
                    hexline += hexnum #'{:0{}X}'.format(hexnum, 2) #"%2.2X" %num,end='')  # f'{data:X}\n'
                #print(hexline)
                outF.write(hexline) #hexline) #"%2.2X" %data,end='')
                #outF.write(hex(int(data, 2))) 
                #print()
                data = inF.read(8)
    #sys.stdout = original_stdout
    outF.close()
    inF.close()
    print('Data written to ' ,outFileName)


def load_data(file):
  data = open(file, "r") #loads file
  data=data.read()       #reads file
  data=data.split("\n") #turn one giant string into a list of line strings. The last line is "" and drops it.

  return data


def sort_into_events(data, address):
  processed_data=[]
  processed_event=[]
  evNbs=[]
  timestamps=[]
  ts=[]
  timeSinceSpill=[]
  tSinceSpill=[]
  print('using "'+address+'" to delimit packet time samples')
  hasAddedFirstEventWord=False
  skipOne=False #avoid setting timestamp using the first few weird lines in the file
  for line in data[0:] :
    try:
      if line == 'FFFFFFFFFFFFFFFF' : 
        processed_data.append(processed_event)
        #print("appended event to data:", processed_event)
        processed_event=[]
        timestamps.append(ts) #add the most recent one to the event 
        skipOne=True
      elif skipOne : 
          #print("Found time since spill line: "+line)
          timeSinceSpill.append(line[8:]) #spill count is only last half of the line
          #print("Stored time since spill: "+tSinceSpill[0]) 
          skipOne=False
      else : #this is the event data 
          processed_event.append(line)
    except:
        #print("except (empty) line") 
        pass #blank lines in data that would otherwise crash program
  print("first event appended to data:", processed_data[0])
  print("first time since spill appended to data:", timeSinceSpill[0])
  print("second time since spill appended to data:", timeSinceSpill[1])
  return processed_data, timeSinceSpill


def sort_into_eventsBin(inFileName, address):
    #not currently used. would avoid the txt file entirely.
    #niramay cautions that that can lead to RAM issues with large number of events
    # could be interesting if used with a maxEvents for shorter dev cycle testing
  processed_data=[]
  processed_event=[]
  evNbs=[]
  timestamps=[]
  ts=[]
  timeSinceSpill=[]
  tSinceSpill=[]
  print('using "'+address+'" to delimit packet time samples')
  hasAddedFirstEventWord=False
  skipOne=True
  with open(inFileName,'rb') as inF :
      line = inF.read(8).hex()
      print(line)
      line = inF.read(8).hex()
      print(line)
      while line:
          try:
              if line == 'ffffffffffffffff' : 
                  processed_data.append(processed_event)
                  print("appended event to data:", processed_event)
                  processed_event=[]
                  timestamps.append(ts) #add the most recent one to the event 
                  skipOne=True #next word is time sample
              elif skipOne : 
                  print("Found time since spill line: "+line)
                  #tSinceSpill=[]
                  timeSinceSpill.append(line[8:]) #spill count is only last half of the line
                  print("Stored time since spill: "+line[8:]) 
                  skipOne=False
              else : #this is data
                  processed_event.append(line)
          except: #all special lines dealt with. just add to event data
              print("empty line "+line) 
              #pass #blank lines in data that would otherwise crash program
          line = inF.read(8).hex()
          print(line)

  print("first event appended to data:", processed_data[0])
  print("first time since spill appended to data:", timeSinceSpill[0])
  print("second time since spill appended to data:", timeSinceSpill[1])
  return processed_data, timeSinceSpill

######### = = = = = = = = = = = = = = = = = 



def main(options,args) :

    inFile=str(options.inFile)
    txtFileName=inFile.replace(".dat",".txt")
    outfilename=str(options.outFile)
    if outfilename == '' :
        outfilename=inFile.replace(".dat","_reformat.dat")
    outfile=open(outfilename, "wb")
    passThrough=bool(options.passThrough)
    trigOnTDC=bool(options.tdcTrig)
    maxEvents=int(options.maxEvents)
    isVerbose=int(options.verbose)
    timeSampleCut =int(options.firstTimeSample)
    samplesPerEvent=int(options.nTimeSamples)

    plotDir="plots/"+outfilename.replace(".dat", "")
    if isVerbose : #only make plots here in verbose mode
        os.makedirs(plotDir, exist_ok = True)
        
    print("inFile = %s,\noutfile = %s,\npassThrough = %i,\ntrigOnTDC = %i,\nmaxEvents = %i,\nverbose = %i,\ntimeSampleCut = %i,\nsamplesPerEvent = %i" % (inFile, outfilename,int(passThrough),int(trigOnTDC),maxEvents,isVerbose,timeSampleCut,samplesPerEvent) )
    #header stuff to do:
    # cap id mismatch has its own flag. could let everything else be some sort of checksum error.
    # this could actually also be some dedicated bits for now; we don't have a checksum
    # protocol in mind.


    binToTxt(inFile, txtFileName)
    data = load_data(txtFileName)
    timeSampleMarkerWord='got read response from 192.168.1.30' #'r' #'some bogus that we will never find (to be removed)'#
    events, spillTime = sort_into_events(data, timeSampleMarkerWord)
    print(events[0])
    eventCounter=0
    subEventCounter=0
#    peds=[7, 5, 7, 6, 7, 5, 5, 7, 6, 7, 6, 5, 6, 7, 5, 6]
    peds=[90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 256, 90]     #use to trigger on high ADC values (margin=10 is added later)

    for event,spillT in zip(events,spillTime):
        prevCapID0=-1 #reset these for every new event, they can be anything..?
        prevCapID1=-1
        print('=========== NEW EVENT ========')
        fiber2=""
        fiber1=""
        event = event[1:]
        for word in event: 
            fiber1+=word[:8]
            fiber2+=word[8:]
        print(fiber1)
        print(fiber2)
        fiber1 = clean_kchar(fiber1)
        fiber1 = clean_BC7C(fiber1)
        fiber2 = clean_kchar(fiber2)
        fiber2 = clean_BC7C(fiber2)
        fiber1 = fiber1.split('BC')
        fiber2 = fiber2.split('BC')

        outValues = []
        #useless definitions already here to avoid errors if there is an empty time sample 
        CRC0error=0
        CRC1error=0
        CIDunsync=0
        CIDskip=0

        #adjust the sub-event counter: subtract any remainder mod samplesPerEvent, to make sure the previous event
        # doesn't count towards the next event's sample collection
        subEventCounter-=subEventCounter%samplesPerEvent
        hasTriggered=passThrough #which is false if we want to actually trigger first
        timeStamp=0  #this is no longer part of the event info. so set it to 0 and keep for compatibility 
        timeStampTick=0  #same here 
        timeSinceSpill=int(spillT,16)
        for i,z in enumerate(zip(fiber1,fiber2)):
            print('---------- NEW TIME SAMPLE ({0}) ----------'.format(i))
            if isVerbose:
                print(z)
            f1 = qie_frame('fiber 1')
            f2 = qie_frame('fiber 2')
            f1.peds = peds[:8]
            f2.peds = peds[8:]
            if i >= timeSampleCut and not f1.decode(z[0]) + f2.decode(z[1]) : 
                #check if either sets hasTriggered. in either case add to a redout "buffer" 
                if not hasTriggered :
                    hasTriggered=f1.triggerForReadout(trigOnTDC)
                if not hasTriggered :
                    hasTriggered=f2.triggerForReadout(trigOnTDC)
                if hasTriggered and isVerbose :
                    print("keeping timesample!")
                adcsOut=[] 
                tdcsOut=[] 
                subEventCounter+=1
                #NOTE that these are now filled regardless of trigger decision
                for j,codes in enumerate(zip(f1.adcs,f1.tdcs)):
                    if isVerbose :
                        hist_pulse[j].Fill(i,codes[0])
                        hist_adc[j].Fill(codes[0])
                        hist_tdc[j].Fill(codes[1])
                    adcsOut.append(codes[0]) #store these here to combine fibers
                    tdcsOut.append(codes[1]) #in order ADC, then TDC
                if isVerbose :
                    f1.triggerPrint()
                #f1.print()
                for j,codes in enumerate(zip(f2.adcs,f2.tdcs)):
                    if isVerbose :
                        hist_pulse[j+8].Fill(i,codes[0])
                        hist_adc[j+8].Fill(codes[0])
                        hist_tdc[j+8].Fill(codes[1])
                    adcsOut.append(codes[0]) 
                    tdcsOut.append(codes[1]) 
                if isVerbose :
                    f2.triggerPrint()
                #f2.print()
                CIDunsync=( f1.capid != f2.capid )
                if CIDunsync :  #useful debugging/header cross check
                    print("CID between fiber1 and 2 unsynced!")
                CRC0error=( f1.ce != 0 )
                if CRC0error :
                    print("fiber 1 has CE error flag!")            
                CRC1error=( f2.ce != 0 )
                if CRC1error :
                    print("fiber 2 has CE error flag!")
                CIDskip=False
                if (prevCapID0 < 0) :
                    prevCapID0 = f1.capid
                else :    #this logic still needs some work: what happens if there is a corrupt time sample word in the middle?
                    if (prevCapID0+1)%4  != (f1.capid)%4 :
                        CIDskip=True
                        print("Found CIDskip in fiber 1! previous CapID: "+str(prevCapID0)+", current: "+str(f1.capid))
                        f1.print()
                prevCapID0=f1.capid #update after checking
                if (prevCapID1 < 0) :
                    prevCapID1 = f2.capid
                else :
                    if (prevCapID1+1)%4 != (f2.capid)%4 :
                        CIDskip=True
                        print("Found CIDskip in fiber 2! previous CapID: "+str(prevCapID1)+", current: "+str(f2.capid))
                        f2.print() 
                prevCapID1=f2.capid #update

                #write these in the right order, now to a single stream per time sample
                outValues.append(adcsOut) 
                outValues.append(tdcsOut) 
                if (subEventCounter % samplesPerEvent == 0) and hasTriggered :
                    # we want to write this chunk as an "event", with event number count,
                    # and the rest of the header, to file 
                    eventCounter+=1
                    print("\t\t------> writing event "+str(eventCounter)+" to file at subevent count: "+str(subEventCounter))
                    endian="little"
                    outfile.write( int(timeStamp).to_bytes(4, byteorder=endian, signed=False)) 
                    outfile.write( int(timeStampTick).to_bytes(4, byteorder=endian, signed=False)) #placeholder number for now, clock ticks?
                    outfile.write( int(timeSinceSpill).to_bytes(4, byteorder=endian, signed=False)) 
                    outfile.write( int(eventCounter).to_bytes(3, byteorder=endian, signed=False)) 
                    errorWord=0
                    errorWord |= (CRC0error << 0)
                    errorWord |= (CRC1error << 1)
                    errorWord |= (CIDunsync << 2)
                    errorWord |= (CIDskip   << 3)
                    if isVerbose:
                        eWord=errorWord.to_bytes(1, byteorder=endian, signed=False)
                        print("\t\t------> writing header: "+str(eventCounter)+", error = "+str(eWord))
                    outfile.write( errorWord.to_bytes(1, byteorder=endian, signed=False)) 
                    if len(outValues) != samplesPerEvent*2 : #1 ADC, 1 TDC vector per time sample 
                        print("UH-OH! got "+str(len(outValues))+" words in the event data!")
                        print(outValues)
                    for vals in outValues :
                        if len(vals) != 16 : #expect one word per channel 
                            print("UH-OH! got "+str(len(vals))+" words in the event data!")
                            print(vals)
                        for word in vals :  #ADC and TDC both 8-bit words in final format
                            outfile.write( int(word).to_bytes(1, byteorder=endian, signed=False)) 
                    outValues.clear()
                    hasTriggered=passThrough #False
                    if i + samplesPerEvent > 31 : #after writing, don't continue looping if remaining time samples won't make a full event 
                        print("Not using time samples after "+str(i)+"; breaking")
                        break #don't use late data 

        #this is all just for printing after the last time sample of every "actual" event
        if isVerbose:
            outWord=''
            outWord+="{0:0{1}X} ".format(eventCounter, 32)
            outWord+=str(int(CRC0error))
            outWord+=str(int(CRC1error))
            outWord+=str(int(CIDunsync))
            outWord+=str(int(CIDskip))
            outWord+=''.join(map(str,outValues))
            print(outWord)

        if maxEvents > 0 and eventCounter >= maxEvents :
            print("Reached maxEvents:", eventCounter)
            break

    outfile.close()
    print("Wrote "+str(eventCounter)+" events to .dat file")

    if isVerbose :
        can = r.TCanvas('can','can',500,500)
        can.SetLogz()
        for i,h in enumerate(hist_pulse):
            h.Draw("colz")
            can.SaveAs('{0}/pulse{1}.png'.format(plotDir, i))
        for i,h in enumerate(hist_adc):
            h.Draw()
            can.SetLogy()
            can.SaveAs('{0}/adc{0}.png'.format(plotDir, i))
        for i,h in enumerate(hist_tdc):
            h.Draw()
            can.SaveAs('{0}/tdc{1}.png'.format(plotDir, i))





if __name__ == "__main__":
    #here: add any option flags needed, and then pick them up in "main" above                                                                 
    parser = OptionParser()
    parser.add_option('-i', '--inFile', dest='inFile', default='', help='input .txt file')
    parser.add_option('-o', '--outFileName', dest='outFile', default='', help='output (.dat) file name (default: .dat --> _reformat.dat)')
    parser.add_option('-p', '--passThrough', dest='passThrough', action='store_true', default=False, help='whether to "trigger"/pass through all events (default: false)')
    parser.add_option('-t', '--tdcTrigger', dest='tdcTrig', action='store_true', default=False, help='whether to "trigger" on TDC < 3 (default: false; trigger is then an ADC threshold)')
    parser.add_option('-a', '--adcTrigger', dest='adcTrig', action='store_true', default=False, help='whether to "trigger" on ADC > pedestal + 10 (default: true)')
    
    parser.add_option('-N', '--maxEvents', dest='maxEvents', default=-1, help='max number of events to process (default: all)')
    parser.add_option('-v', '--verbose', dest='verbose', action='store_true', default=False, help='to make a lot of verbose printouts (default: false)')
    parser.add_option('-n', '--nTimeSamples', dest='nTimeSamples', default=30, help='number of time samples per event (default: 30)')
    parser.add_option('-f', '--firstTimeSample', dest='firstTimeSample', default=0, help='first time sample to consider when accumulating samples to an event (default: 6)')
    
    
    (options, args) = parser.parse_args()
    if options.inFile == '' :
        print("Have to specify an input file (option -i)")
        sys.exit(0)
    if options.adcTrig : #not actually used in main() but overrides others
        options.passThrough=False
        options.tdcTrig=False
    if (options.tdcTrig or options.adcTrig) and options.passThrough : # make user choose.
        #in effect, only the combination -t and -p will trigger this message
        print("Can't choose both -p for passthrough (= no trigger) and -a for adc trigger or -t for tdc trigger at the same time. Pick one.")
        sys.exit(0)
    if not (options.tdcTrig or options.adcTrig or options.passThrough) : # make user choose.
        print("Must choose one mode: -p for passthrough (= no trigger)/-a for adc trigger/-t for tdc trigger.")
        sys.exit(0)
    main(options,args)
    
