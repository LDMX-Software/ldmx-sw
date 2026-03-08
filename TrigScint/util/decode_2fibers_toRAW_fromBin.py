import os

#from sys import argv
import sys
from optparse import OptionParser

import ROOT as r


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
        print(f'CAPID: {self.capid} CE: {self.ce} BC0: {self.bc0}')
        print('adcs: ',' '.join(map(str,self.adcs)))
        print('tdcs: ',' '.join(map(str,self.tdcs)))

    def triggerPrint(self):
        print_frame=False
        for adc in self.adcs:
            if adc > 100 and adc < 255:
                print_frame=True
                break
        for tdc in self.tdcs:
            if tdc != 3 :
                print_frame=True
                break
        if (print_frame) :
            print('[[qie_frame]] Trigger')
            self.print()
        return

    def triggerForReadout(self, do_tdc_trig) :
        if do_tdc_trig : # trigger on good tdc somewhere in the event
            for tdc in self.tdcs :
                if tdc < 3 :
                    print("triggering with tdc = "+str(tdc))
                    return True
        else :
            margin = 10 #adc counts
            for adc,ped in zip(self.adcs, self.peds, strict=False):
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
    hist_pulse.append(r.TH2F(
        f"pulse{i}", f"pulse{i};time sample;ADC",
        tsmax, -0.5, tsmax-0.5, adcmax, -0.5, adcmax-0.5))

hist_adc=[]
for i in range(16):
    hist_adc.append(r.TH1F(f"adc{i}",f"adc{i};ADC;Arbitrary",adcmax,-0.5,adcmax-0.5))

hist_tdc=[]
for i in range(16):
    hist_tdc.append(r.TH1F(f"tdc{i}",f"tdc{i};TDC;Arbitrary",4,-0.5,3.5))


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


def binToTxt(in_file_name, out_file_name):
    print('Processing file ',in_file_name)
    with open(out_file_name,'w') as out_f:
 #       sys.stdout = out_f
        with open(in_file_name,'rb') as in_f:
            data = in_f.read(8)
            while data:
                hexline='\n'
                for dd in data:  #convert byte per byte
                    #format in hex with padding (don't lose leading zeroes)
                    hexnum=f"{dd:0>2X}" #int(hexnum,2))
                    #print(hexnum)
                    #'{:0{}X}'.format(hexnum, 2) #"%2.2X" %num,end='')  # f'{data:X}\n'
                    hexline += hexnum
                #print(hexline)
                out_f.write(hexline) #hexline) #"%2.2X" %data,end='')
                #out_f.write(hex(int(data, 2)))
                #print()
                data = in_f.read(8)
    #sys.stdout = original_stdout
    out_f.close()
    in_f.close()
    print('Data written to ' ,out_file_name)


def load_data(file):
  data = open(file) #loads file
  data=data.read()       #reads file
  #turn one giant string into a list of line strings. The last line is "" and drops it.
  data=data.split("\n")

  return data


def sort_into_events(data, address):
  processed_data=[]
  processed_event=[]
  # ev_nbs=[]
  timestamps=[]
  ts=[]
  time_since_spill=[]
  # t_since_spill=[]
  print('using "'+address+'" to delimit packet time samples')
  # has_added_first_event_word=False
  skip_one=False #avoid setting timestamp using the first few weird lines in the file
  for line in data[0:] :
    try:
      if line == 'FFFFFFFFFFFFFFFF' :
        processed_data.append(processed_event)
        #print("appended event to data:", processed_event)
        processed_event=[]
        timestamps.append(ts) #add the most recent one to the event
        skip_one=True
      elif skip_one :
          #print("Found time since spill line: "+line)
          time_since_spill.append(line[8:]) #spill count is only last half of the line
          #print("Stored time since spill: "+t_since_spill[0])
          skip_one=False
      else : #this is the event data
          processed_event.append(line)
    except:
        #print("except (empty) line")
        pass #blank lines in data that would otherwise crash program
  print("first event appended to data:", processed_data[0])
  print("first time since spill appended to data:", time_since_spill[0])
  print("second time since spill appended to data:", time_since_spill[1])
  return processed_data, time_since_spill


def sort_into_eventsBin(in_file_name, address):
    #not currently used. would avoid the txt file entirely.
    #niramay cautions that that can lead to RAM issues with large number of events
    # could be interesting if used with a max_events for shorter dev cycle testing
  processed_data=[]
  processed_event=[]
  # ev_nbs=[]
  timestamps=[]
  ts=[]
  time_since_spill=[]
  # t_since_spill=[]
  print('using "'+address+'" to delimit packet time samples')
  # has_added_first_event_word=False
  skip_one=True
  with open(in_file_name,'rb') as in_f :
      line = in_f.read(8).hex()
      print(line)
      line = in_f.read(8).hex()
      print(line)
      while line:
          try:
              if line == 'ffffffffffffffff' :
                  processed_data.append(processed_event)
                  print("appended event to data:", processed_event)
                  processed_event=[]
                  timestamps.append(ts) #add the most recent one to the event
                  skip_one=True #next word is time sample
              elif skip_one :
                  print("Found time since spill line: "+line)
                  #t_since_spill=[]
                  #spill count is only last half of the line
                  time_since_spill.append(line[8:])
                  print("Stored time since spill: "+line[8:])
                  skip_one=False
              else : #this is data
                  processed_event.append(line)
          except: #all special lines dealt with. just add to event data
              print("empty line "+line)
              #pass #blank lines in data that would otherwise crash program
          line = in_f.read(8).hex()
          print(line)

  print("first event appended to data:", processed_data[0])
  print("first time since spill appended to data:", time_since_spill[0])
  print("second time since spill appended to data:", time_since_spill[1])
  return processed_data, time_since_spill

######### = = = = = = = = = = = = = = = = =



def main(options,args) :

    in_file=str(options.in_file)
    txt_file_name=in_file.replace(".dat",".txt")
    outfilename=str(options.outFile)
    if outfilename == '' :
        outfilename=in_file.replace(".dat","_reformat.dat")
    outfile=open(outfilename, "wb")
    pass_through=bool(options.pass_through)
    trig_on_tdc=bool(options.tdcTrig)
    max_events=int(options.max_events)
    is_verbose=int(options.verbose)
    time_sample_cut =int(options.firstTimeSample)
    samples_per_event=int(options.n_time_samples)

    plot_dir="plots/"+outfilename.replace(".dat", "")
    if is_verbose : #only make plots here in verbose mode
        os.makedirs(plot_dir, exist_ok = True)

    print(
        "in_file = %s,\noutfile = %s,\npassThrough = %i,\ntrigOnTDC = %i,"
        "\nmaxEvents = %i,\nverbose = %i,\ntimeSampleCut = %i,"
        "\nsamplesPerEvent = %i" % (
            in_file, outfilename, int(pass_through), int(trig_on_tdc),
            max_events, is_verbose, time_sample_cut, samples_per_event)
    )
    #header stuff to do:
    # cap id mismatch has its own flag. could let everything else be some sort of
    # checksum error.
    # this could actually also be some dedicated bits for now; we don't have a checksum
    # protocol in mind.


    binToTxt(in_file, txt_file_name)
    data = load_data(txt_file_name)
    time_sample_marker_word = (
        'got read response from 192.168.1.30'
        #'r' #'some bogus that we will never find (to be removed)'#
    )
    events, spill_time = sort_into_events(data, time_sample_marker_word)
    print(events[0])
    event_counter=0
    sub_event_counter=0
#    peds=[7, 5, 7, 6, 7, 5, 5, 7, 6, 7, 6, 5, 6, 7, 5, 6]
    peds=[90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 256, 90]
    #use to trigger on high ADC values (margin=10 is added later)

    for event,spill_t in zip(events,spill_time, strict=False):
        prev_cap_id0=-1 #reset these for every new event, they can be anything..?
        prev_cap_id1=-1
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

        out_values = []
        #useless definitions already here to avoid errors if there is an empty time
        #sample
        crc0_error=0
        crc1_error=0
        cid_unsync=0
        cid_skip=0

        #adjust the sub-event counter: subtract any remainder mod samples_per_event, to
        #make sure the previous event
        # doesn't count towards the next event's sample collection
        sub_event_counter-=sub_event_counter%samples_per_event
        has_triggered=pass_through #which is false if we want to actually trigger first
        time_stamp=0  #this is no longer part of the event info.
        #set it to 0 and keep for compatibility
        time_stamp_tick=0  #same here
        time_since_spill=int(spill_t,16)
        for i,z in enumerate(zip(fiber1,fiber2, strict=False)):
            print(f'---------- NEW TIME SAMPLE ({i}) ----------')
            if is_verbose:
                print(z)
            f1 = qie_frame('fiber 1')
            f2 = qie_frame('fiber 2')
            f1.peds = peds[:8]
            f2.peds = peds[8:]
            if i >= time_sample_cut and not f1.decode(z[0]) + f2.decode(z[1]) :
                #check if either sets has_triggered. in either case add to a redout
                #"buffer"
                if not has_triggered :
                    has_triggered=f1.triggerForReadout(trig_on_tdc)
                if not has_triggered :
                    has_triggered=f2.triggerForReadout(trig_on_tdc)
                if has_triggered and is_verbose :
                    print("keeping timesample!")
                adcs_out=[]
                tdcs_out=[]
                sub_event_counter+=1
                #NOTE that these are now filled regardless of trigger decision
                for j,codes in enumerate(zip(f1.adcs,f1.tdcs, strict=False)):
                    if is_verbose :
                        hist_pulse[j].Fill(i,codes[0])
                        hist_adc[j].Fill(codes[0])
                        hist_tdc[j].Fill(codes[1])
                    adcs_out.append(codes[0]) #store these here to combine fibers
                    tdcs_out.append(codes[1]) #in order ADC, then TDC
                if is_verbose :
                    f1.triggerPrint()
                #f1.print()
                for j,codes in enumerate(zip(f2.adcs,f2.tdcs, strict=False)):
                    if is_verbose :
                        hist_pulse[j+8].Fill(i,codes[0])
                        hist_adc[j+8].Fill(codes[0])
                        hist_tdc[j+8].Fill(codes[1])
                    adcs_out.append(codes[0])
                    tdcs_out.append(codes[1])
                if is_verbose :
                    f2.triggerPrint()
                #f2.print()
                cid_unsync=( f1.capid != f2.capid )
                if cid_unsync :  #useful debugging/header cross check
                    print("CID between fiber1 and 2 unsynced!")
                crc0_error=( f1.ce != 0 )
                if crc0_error :
                    print("fiber 1 has CE error flag!")
                crc1_error=( f2.ce != 0 )
                if crc1_error :
                    print("fiber 2 has CE error flag!")
                cid_skip=False
                if (prev_cap_id0 < 0) :
                    prev_cap_id0 = f1.capid
                else :    #this logic still needs some work: what happens if
                    #there is a corrupt time sample word in the middle?
                    if (prev_cap_id0+1)%4  != (f1.capid)%4 :
                        cid_skip=True
                        print("Found cid_skip in fiber 1! previous CapID: "
                              + str(prev_cap_id0) + ", current: "
                              + str(f1.capid))
                        f1.print()
                prev_cap_id0=f1.capid #update after checking
                if (prev_cap_id1 < 0) :
                    prev_cap_id1 = f2.capid
                else :
                    if (prev_cap_id1+1)%4 != (f2.capid)%4 :
                        cid_skip=True
                        print("Found cid_skip in fiber 2! previous CapID: "
                              + str(prev_cap_id1) + ", current: "
                              + str(f2.capid))
                        f2.print()
                prev_cap_id1=f2.capid #update

                #write these in the right order, now to a single stream per time sample
                out_values.append(adcs_out)
                out_values.append(tdcs_out)
                if (sub_event_counter % samples_per_event == 0) and has_triggered :
                    # we want to write this chunk as an "event", with event number
                    # count,
                    # and the rest of the header, to file
                    event_counter+=1
                    print("\t\t------> writing event " + str(event_counter)
                          + " to file at subevent count: "
                          + str(sub_event_counter))
                    endian="little"
                    outfile.write(
                        int(time_stamp).to_bytes(4, byteorder=endian,
                                                 signed=False))
                    outfile.write(
                        int(time_stamp_tick).to_bytes(
                            4, byteorder=endian, signed=False)
                    ) #placeholder number for now, clock ticks?
                    outfile.write(
                        int(time_since_spill).to_bytes(4, byteorder=endian,
                                                       signed=False))
                    outfile.write(
                        int(event_counter).to_bytes(3, byteorder=endian,
                                                    signed=False))
                    error_word=0
                    error_word |= (crc0_error << 0)
                    error_word |= (crc1_error << 1)
                    error_word |= (cid_unsync << 2)
                    error_word |= (cid_skip   << 3)
                    if is_verbose:
                        e_word=error_word.to_bytes(1, byteorder=endian, signed=False)
                        print("\t\t------> writing header: "
                              + str(event_counter) + ", error = "
                              + str(e_word))
                    outfile.write(
                        error_word.to_bytes(1, byteorder=endian, signed=False))
                    #1 ADC, 1 TDC vector per time sample
                    if len(out_values) != samples_per_event*2 :
                        print("UH-OH! got " + str(len(out_values))
                              + " words in the event data!")
                        print(out_values)
                    for vals in out_values :
                        if len(vals) != 16 : #expect one word per channel
                            print("UH-OH! got " + str(len(vals))
                                  + " words in the event data!")
                            print(vals)
                        for word in vals :  #ADC and TDC both 8-bit words
                            outfile.write(
                                int(word).to_bytes(1, byteorder=endian,
                                                   signed=False))
                    out_values.clear()
                    has_triggered=pass_through #False
                    #after writing, don't continue looping if remaining
                    #time samples won't make a full event
                    if i + samples_per_event > 31 :
                        print("Not using time samples after "+str(i)+"; breaking")
                        break #don't use late data

        #this is all just for printing after the last time sample of every "actual"
        #event
        if is_verbose:
            out_word=''
            out_word+="{0:0{1}X} ".format(event_counter, 32)
            out_word+=str(int(crc0_error))
            out_word+=str(int(crc1_error))
            out_word+=str(int(cid_unsync))
            out_word+=str(int(cid_skip))
            out_word+=''.join(map(str,out_values))
            print(out_word)

        if max_events > 0 and event_counter >= max_events :
            print("Reached max_events:", event_counter)
            break

    outfile.close()
    print("Wrote "+str(event_counter)+" events to .dat file")

    if is_verbose :
        can = r.TCanvas('can','can',500,500)
        can.SetLogz()
        for i,h in enumerate(hist_pulse):
            h.Draw("colz")
            can.SaveAs(f'{plot_dir}/pulse{i}.png')
        for i,h in enumerate(hist_adc):
            h.Draw()
            can.SetLogy()
            can.SaveAs(f'{plot_dir}/adc{plot_dir}.png')
        for i,h in enumerate(hist_tdc):
            h.Draw()
            can.SaveAs(f'{plot_dir}/tdc{i}.png')





if __name__ == "__main__":
    #here: add any option flags needed, and then pick them up in "main" above
    parser = OptionParser()
    parser.add_option('-i', '--in_file', dest='in_file', default='',
                      help='input .txt file')
    parser.add_option('-o', '--out_file_name', dest='outFile', default='',
                      help='output (.dat) file name (default: .dat --> _reformat.dat)')
    parser.add_option('-p', '--pass_through', dest='pass_through',
                      action='store_true', default=False,
                      help='whether to "trigger"/pass through all events'
                           ' (default: false)')
    parser.add_option('-t', '--tdcTrigger', dest='tdcTrig',
                      action='store_true', default=False,
                      help='whether to "trigger" on TDC < 3 (default: false;'
                           ' trigger is then an ADC threshold)')
    parser.add_option('-a', '--adcTrigger', dest='adcTrig',
                      action='store_true', default=False,
                      help='whether to "trigger" on ADC > pedestal + 10'
                           ' (default: true)')

    parser.add_option('-N', '--max_events', dest='max_events', default=-1,
                      help='max number of events to process (default: all)')
    parser.add_option('-v', '--verbose', dest='verbose',
                      action='store_true', default=False,
                      help='to make a lot of verbose printouts (default: false)')
    parser.add_option('-n', '--n_time_samples', dest='n_time_samples',
                      default=30,
                      help='number of time samples per event (default: 30)')
    parser.add_option('-f', '--firstTimeSample', dest='firstTimeSample',
                      default=0,
                      help='first time sample to consider when accumulating'
                           ' samples to an event (default: 6)')


    (options, args) = parser.parse_args()
    if options.in_file == '' :
        print("Have to specify an input file (option -i)")
        sys.exit(0)
    if options.adcTrig : #not actually used in main() but overrides others
        options.pass_through=False
        options.tdcTrig=False
    if (options.tdcTrig or options.adcTrig) and options.pass_through :
        # make user choose.
        #in effect, only the combination -t and -p will trigger this message
        print("Can't choose both -p for passthrough (= no trigger) and"
              " -a for adc trigger or -t for tdc trigger at the same time."
              " Pick one.")
        sys.exit(0)
    if not (options.tdcTrig or options.adcTrig or options.pass_through) :
        # make user choose.
        print("Must choose one mode: -p for passthrough (= no trigger)"
              "/-a for adc trigger/-t for tdc trigger.")
        sys.exit(0)
    main(options,args)
