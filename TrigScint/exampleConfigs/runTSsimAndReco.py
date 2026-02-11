import sys
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

this_pass_name="sim"
p = ldmxcfg.Process(this_pass_name)

from LDMX.Hcal import hcal_geometry
#from LDMX.Ecal import ecal_geometry

n_events = 4000
kill_chan_8 = True # toggle to kill all signal from channel 8 (dead in testbeam)
n_electrons=2
beam_energy=4.  #in GeV

n_channels=12
gain_list=[2e6]*n_channels
ped_list=[6.]*n_channels

if kill_chan_8 :
    gain_list[8]=gain_list[8]*gain_list[8]  #to make the hit PEs very small
    kill_string="killChan8_"


gun_z_pos=800 #3000  #mm -- define as positive here, for file naming; set sign below
det_v=2.0        #detector geometry version number
beam_x_smear=7.5 #mm    7.5mm for reasonable efficiency, larger than TS module to get more empty (no MIP)  events. set to 150mm for large noise
beam_y_smear=20  #mm    20 mm      -- " --, set to 200 for large noise
noise_per_event=0.08  #average number of PEs from SiPM noise per event (gets scaled by n_time_samples to be constant) -- 0.1 per event is taken from run183
start_sample=15

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

p.run = 1
p.max_events = n_events
p.output_files = ['testbeamSim_'+str(n_electrons)+'e_zNeg'+str(gun_z_pos)+'mm_beamSpot'+str(beam_x_smear)+'x'+str(beam_y_smear)+'mm_'+str(n_time_samples)+'tSamp_eNoise'+str(elec_noise)+'_tauInv'+str(k_expo)+'_detV'+str(det_v)+'_'+kill_string+str(p.max_events)+'ev.root']
print("Producing output file: "+p.output_files[0])

gun_z_pos   =-float(gun_z_pos)  #get sign right, and make floats, to use as parameters
beam_x_smear=float(beam_x_smear)
beam_y_smear=float(beam_y_smear)

#gun_z_pos=-1404. #mm
#gun_z_pos=-1504. #mm
#gun_z_pos=-1204. #mm

#------ set up beam simulation -------


mpg_gen = generators.multi( "mgpGen" ) # this is the line that actually creates the generator
mpg_gen.vertex = [ 0., 0., gun_z_pos ] # mm
mpg_gen.n_particles = n_electrons
mpg_gen.pdg_id = 11
mpg_gen.enable_poisson = False #True
mpg_gen.momentum = [ 0., 0., beam_energy ]

gun = generators.gun('particle_gun')
gun.particle = 'e-'
gun.direction = [0., 0., 1.]
gun.position = [0., 0., gun_z_pos]
gun.energy = beam_energy   #gev

simulation = simulator.simulator('test_TS')
simulation.generators=[mpg_gen] #gun]
simulation.setDetector('ldmx-hcal-prototype-v'+str(det_v))
#simulation.setDetector('ldmx-hcal-prototype-v1.0')
simulation.beamSpotSmear = [beam_x_smear, beam_y_smear, 0] #mm, at start position
simulation.description = "Inclusive "+str(beam_energy)+" GeV electron events, "+str(n_electrons)+"e"


#### the digi and tb hit step not fully implemented !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#------ set up digitization -------

from LDMX.TrigScint.trigScint import TrigScintQIEDigiProducer

ts_digis = TrigScintQIEDigiProducer.pad1() #up() #
ts_digis.input_collection= "TriggerPadUpSimHits"
ts_digis.number_of_strips = 12
ts_digis.mean_noise=float(noise_per_event/n_time_samples)

ts_digis.maxts = n_time_samples
ts_digis.elec_noise = elec_noise
ts_digis.expo_k = k_expo
ts_digis.sipm_gain = 2.e6
ts_digis.zeroSupp_in_pe=0.5
ts_digis.toff_overall = 25.*start_sample
ts_digis.pe_per_mip = 110. #from fit to 2-hit clusters in data run 183, April4 2310, all plastic


#------ set up event readout linearization -------

from LDMX.TrigScint.trigScint import EventReadoutProducer

ts_ev=EventReadoutProducer("eventLinearizer")
ts_ev.input_pass_name=this_pass_name
ts_ev.input_collection=ts_digis.output_collection #"trigScintQIEDigisPad1"
ts_ev.time_shift=0 #time_offset
ts_ev.verbose = True

#------ set up hit reconstruction -------

from LDMX.TrigScint.trigScint import TestBeamHitProducer

tb_hits  =TestBeamHitProducer("tb_hits")
tb_hits.input_pass_name=this_pass_name
#if input_pass_name=="sim" : #different conventions in sim and data
#    tb_hits.input_collection="trigScintQIEDigisPad1"
#else :
tb_hits.input_collection="QIEsamplesPad1"
tb_hits.pedestals=ped_list
tb_hits.gain=gain_list
tb_hits.start_sample=start_sample
tb_hits.pulse_width=12 #5
tb_hits.pulse_width_lyso=12
tb_hits.do_clean_hits=True
tb_hits.n_instrumented_channels=n_channels


#------ set up clustering -------

from LDMX.TrigScint.trigScint import TestBeamClusterProducer

tb_clusters  =TestBeamClusterProducer("tb_clusters")
tb_clusters.input_pass_name=this_pass_name
tb_clusters.input_collection=tb_hits.output_collection #"trigScintQIEDigisPad1" #
tb_clusters.pad_time=100.
tb_clusters.time_tolerance=999.
tb_clusters.verbosity=0
tb_clusters.clustering_threshold = 50.  #to add in neighboring

tb_clusters_3  =TestBeamClusterProducer("tb_clusters_3")
tb_clusters_3.input_pass_name=this_pass_name
tb_clusters_3.input_collection=tb_hits.output_collection
tb_clusters_3.output_collection=tb_clusters.output_collection+"ThreeHits"
tb_clusters_3.max_cluster_width=3
tb_clusters_3.pad_time=100.
tb_clusters_3.time_tolerance=999.
tb_clusters_3.verbosity=0
tb_clusters_3.clustering_threshold = 50.  #to add in neighboring


#### ---- analyzers ------

from LDMX.TrigScint.trigScint import QIEAnalyzer

ts_ana=QIEAnalyzer("plotMaker")
ts_ana.input_pass_name=this_pass_name
ts_ana.start_sample=0
ts_ana.pedestals=ped_list
ts_ana.gain=gain_list


from LDMX.TrigScint.trigScint import TestBeamClusterAnalyzer

cl_ana_2hit = TestBeamClusterAnalyzer("2-hitClusters")
cl_ana_2hit.input_collection=tb_clusters.output_collection

cl_ana_3hit = TestBeamClusterAnalyzer("3-hitClusters")
cl_ana_3hit.input_collection=tb_clusters_3.output_collection



outname=p.output_files[0].replace(".root", "_plots.root")
#p.output_files = [ outname ]
p.histogram_file = outname

p.sequence=[simulation,
            ts_digis,
            ts_ev,
            tb_hits,
            tb_clusters_3,
            tb_clusters,
            #cl_ana_2hit,
            #cl_ana_3hit
            #      ts_ev,
            #      ts_ana
                  ]


p.term_log_level = 2
