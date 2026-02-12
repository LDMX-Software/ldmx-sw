import sys
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator


this_pass_name="sim"
p = ldmxcfg.Process(this_pass_name)

gun_z_pos=1100 #3000  #mm -- define as positive here, for file naming; set sign below
det_v=2        #detector geometry version number
beam_x_smear=7.5 #mm
beam_y_smear=20 #mm
noise_per_event=1.  #average number of PEs from SiPM noise per event (gets scaled by n_time_samples to be constant)
start_sample=17.
if len(sys.argv) > 1 :
    n_time_samples=int(sys.argv[1])
else :
    n_time_samples=30 #config default is 5
if len(sys.argv) > 2 :
    elec_noise=float(sys.argv[2])
else :
    elec_noise=1.5 #config default is 1.5
if len(sys.argv) > 3 :
    k_expo=float(sys.argv[3])
else :
    k_expo=0.1   #config default is 0.1

p.run = 10
p.max_events = 2000
p.output_files = ['testbeamSim_zNeg'+str(gun_z_pos)+'mm_beamSpot'+str(beam_x_smear)+'x'+str(beam_y_smear)+'mm_'+str(n_time_samples)+'tSamp_eNoise'+str(elec_noise)+'_tauInv'+str(k_expo)+'_detV'+str(det_v)+'_'+str(p.max_events)+'ev.root']
print("Producing output file: "+p.output_files[0])

gun_z_pos   =-float(gun_z_pos)  #get sign right, and make floats, to use as parameters
beam_x_smear=float(beam_x_smear)
beam_y_smear=float(beam_y_smear)

gun = generators.gun('particle_gun')
gun.particle = 'e-'
gun.direction = [0., 0., 1.]
gun.position = [0., 0., gun_z_pos]
gun.energy = 4.   #gev

simulation = simulator.simulator('test_TS')
simulation.generators=[gun]
simulation.setDetector('ldmx-hcal-prototype-v'+str(det_v)+'.0')
simulation.beamSpotSmear = [beam_x_smear, beam_y_smear, 0] #mm, at start position

from LDMX.TrigScint.trigScint import TrigScintQIEDigiProducer
from LDMX.TrigScint.trigScint import TrigScintRecHitProducer
from LDMX.TrigScint.trigScint import TrigScintClusterProducer

ts_digis = TrigScintQIEDigiProducer.up()
ts_digis.number_of_strips = 12
ts_digis.mean_noise=float(noise_per_event/n_time_samples)
ts_digis.maxts = n_time_samples
ts_digis.elec_noise = elec_noise
ts_digis.expo_k = k_expo
ts_digis.sipm_gain = 2.e6
ts_digis.zeroSupp_in_pe=0.5
ts_digis.toff_overall = 25.*start_sample
ts_digis.pe_per_mip = 110. #from fit to 2-hit clusters in data run 183, April4 2310, all plastic

ts_rec_hits = TrigScintRecHitProducer.up()
ts_rec_hits.pe_per_mip = ts_digis.pe_per_mip #pick it up here too
ts_rec_hits.gain = ts_digis.gain # same


ts_cl = TrigScintClusterProducer.up()
ts_cl.input_collection = ts_rec_hits.output_collection
ts_cl.pad_time = 100.
ts_cl.time_tolerance = 999.
#ts_cl.verbosity = 3
ts_cl.clustering_threshold = 30.  #to add in neighboring
ts_cl.seed_threshold = 40.



from LDMX.TrigScint.trigScint import EventReadoutProducer

ts_ev=EventReadoutProducer("eventLinearizer")
ts_ev.input_pass_name=this_pass_name
ts_ev.input_collection=ts_digis.output_collection
ts_ev.time_shift=0 #time_offset

n_channels=12
gain_list=[ts_digis.sipm_gain]*n_channels
ped_list=[ts_digis.pedestal]*n_channels


from LDMX.TrigScint.trigScint import QIEAnalyzer

ts_ana=QIEAnalyzer("plotMaker")
ts_ana.input_pass_name=this_pass_name
ts_ana.start_sample=0
ts_ana.pedestals=ped_list
ts_ana.gain=gain_list

outname=p.output_files[0].replace(".root", "_plots.root")
p.histogram_file = outname

p.sequence=[simulation,
                  ts_digis,
                  ts_rec_hits,
                  ts_cl,
                  ts_ev,
                  ts_ana
                  ]


p.term_log_level = 2 #0
