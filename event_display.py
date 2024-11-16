from sklearn import metrics
import numpy as np
import matplotlib.pyplot as plt
def drawcirc(eposition,emomentum,maxHitZ,radius):
    theta = np.linspace(0, 2 * np.pi, 201)
    maxHit = projection(eposition,emomentum,maxHitZ) #where the electron is during the maxhit layer
    circlex = maxHit[0]+radius*np.cos(theta)
    circley = maxHit[1]+radius*np.sin(theta)
    circlez = theta*0 + maxHitZ
    return circlex,circley,circlez
ecal_front_z = 240.
Si_zpos = [7.932,   14.532,  32.146,  40.746,  58.110, 
           67.710,  86.574,  96.774,  115.638, 125.838,
           144.702, 154.902, 173.766, 183.966, 202.830,
           213.030, 231.894, 242.094, 260.958, 271.158,
           290.022, 300.222, 319.086, 329.286, 351.650,
           365.250, 387.614, 401.214, 423.578, 437.178,
           459.542, 473.142, 495.506, 509.106]
ecal_layerZs = ecal_front_z + np.array(Si_zpos)


recoil_tracker_front_z = 7.5
recoil_tracker_zpos = [0.0, 15.0, 30.0, 45, 82.5, 172.5]
recoil_tracker_layerZs = recoil_tracker_front_z + np.array(recoil_tracker_zpos)

tagger_tracker_first_z = -607.5
tagger_tracker_zpos = [0.0, 100.0, 200.0, 300.0, 400.0, 500.0, 600.0]
tagger_tracker_layerZs = tagger_tracker_first_z + np.array(tagger_tracker_zpos)

clearance = 0.001
# Thickness of scoring planes
sp_thickness = 0.001

# Target GDML
# Position
target_z = 0.0
# Tungsten X0 = .3504 cm
# Target thickness = .1X0
target_thickness = 0.3504
# Target dimensions
target_dim_x = 40.
target_dim_y = 100.

# Surround the target with scoring planes
sp_target_down_z = target_z + target_thickness/2 + sp_thickness/2 + clearance
sp_target_up_z = target_z - target_thickness/2 - sp_thickness/2 - clearance

# Trigger scintillator GDML
# Trigger scintillator positions
trigger_pad_thickness = 4.5
trigger_pad_bar_thickness = 2
trigger_pad_bar_gap = 0.3
trigger_pad_dim_x = target_dim_x
trigger_pad_dim_y = target_dim_y
trigger_bar_dx = 40
trigger_bar_dy = 3
number_of_bars = 25

trigger_pad_offset = (target_dim_y - (number_of_bars*trigger_bar_dy + (number_of_bars - 1)*trigger_pad_bar_gap))/2

# Trigger pad distance from the target is -2.4262mm
trigger_pad_up_z = target_z - (target_thickness/2) - (trigger_pad_thickness/2) - clearance
# Trigger pad distance from the target is 2.4262mm
trigger_pad_down_z = target_z + (target_thickness/2) + (trigger_pad_thickness/2) + clearance

# Place scoring planes downstream of each trigger scintillator array
sp_trigger_pad_down_l1_z = trigger_pad_down_z - trigger_pad_bar_gap/2 + sp_thickness/2 + clearance
sp_trigger_pad_down_l2_z = trigger_pad_down_z + trigger_pad_bar_gap/2 + trigger_pad_bar_thickness + sp_thickness/2 + clearance
sp_trigger_pad_up_l1_z = trigger_pad_up_z - trigger_pad_bar_gap/2 + sp_thickness/2 + clearance
sp_trigger_pad_up_l2_z = trigger_pad_up_z + trigger_pad_bar_gap/2 + trigger_pad_bar_thickness + sp_thickness/2 + clearance

# ECal GDML
# ECal layer thicknesses
Wthick_A_dz = 0.75
W_A_dz = 0.75
Wthick_B_dz = 2.25
W_B_dz = 1.5
Wthick_C_dz = 3.5
W_C_dz = 1.75
Wthick_D_dz = 7.0
W_D_dz = 3.5
CFMix_dz = 0.05
CFMixThick_dz = 0.2
PCB_dz = 1.5
Si_dz = 0.5
C_dz = 0.5
Al_dz = 2.0

# Air separating sheets of Al or W with PCB motherboard
# Limited by construction abilities 
FrontTolerance = 0.5

# Gap between layers
BackTolerance = 0.5

# Air separating PCBs from PCB MotherBoards
PCB_Motherboard_Gap = 2.3

# Air separating Carbon sheets in the middle of a layer
CoolingAirGap = 4.0

# Preshower thickness is 20.1mm
preshower_Thickness = Al_dz + FrontTolerance + PCB_dz + PCB_Motherboard_Gap\
    + PCB_dz + CFMix_dz + Si_dz + CFMixThick_dz + CoolingAirGap\
        + 2.*C_dz + CFMixThick_dz + Si_dz + CFMix_dz + PCB_dz\
            + PCB_Motherboard_Gap + PCB_dz + BackTolerance

# Layer A thickness is 20.35mm
layer_A_Thickness = Wthick_A_dz + FrontTolerance + PCB_dz + PCB_Motherboard_Gap\
    + PCB_dz + CFMix_dz + Si_dz + CFMixThick_dz + W_A_dz + C_dz\
        + CoolingAirGap + C_dz + W_A_dz + CFMixThick_dz + Si_dz\
            + CFMix_dz + PCB_dz + PCB_Motherboard_Gap + PCB_dz\
                + BackTolerance

# Layer B thickness is 22.35mm
# GDML comment indicates that this is 22.35mm, but the actual value is 23.35mm!
layer_B_Thickness = Wthick_B_dz + FrontTolerance + PCB_dz + PCB_Motherboard_Gap\
    + PCB_dz + CFMix_dz + Si_dz + CFMixThick_dz + W_B_dz + C_dz\
        + CoolingAirGap + C_dz + W_B_dz + CFMixThick_dz + Si_dz\
            + CFMix_dz + PCB_dz + PCB_Motherboard_Gap + PCB_dz\
                + BackTolerance

# Layer C thickness is 25.1mm
layer_C_Thickness = Wthick_C_dz + FrontTolerance + PCB_dz + PCB_Motherboard_Gap\
    + PCB_dz + CFMix_dz + Si_dz + CFMixThick_dz + W_C_dz + C_dz\
        + CoolingAirGap + C_dz + W_C_dz + CFMixThick_dz + Si_dz\
            + CFMix_dz + PCB_dz + PCB_Motherboard_Gap + PCB_dz\
                + BackTolerance

# Layer D thickness is 32.1mm
layer_D_Thickness = Wthick_D_dz + FrontTolerance + PCB_dz + PCB_Motherboard_Gap\
    + PCB_dz + CFMix_dz + Si_dz + CFMixThick_dz + W_D_dz + C_dz\
        + CoolingAirGap + C_dz + W_D_dz + CFMixThick_dz + Si_dz\
            + CFMix_dz + PCB_dz + PCB_Motherboard_Gap + PCB_dz\
                + BackTolerance

# Number of layers
ecal_A_layers = 1
ecal_B_layers = 1
ecal_C_layers = 9
ecal_D_layers = 5

# ECal thickness is 449.2mm
# GDML comment indicates that this is 449.2mm, but the actual value is 450.2mm!
ECal_dz = preshower_Thickness\
    + layer_A_Thickness*ecal_A_layers\
        + layer_B_Thickness*ecal_B_layers\
            + layer_C_Thickness*ecal_C_layers\
            + layer_D_Thickness*ecal_D_layers

sqrt3 = np.sqrt(3)

module_gap = 1.5  # Flat-to-flat gap between modules
module_radius = 85.  # Center-to-flat radius of one module

# ECal width and height
ECal_dx = module_radius*10./sqrt3 + sqrt3*module_gap
ECal_dy = module_radius*6. + module_gap*2.

# Distance from target to the ECal parent volume
# The calorimeter is an additional .5mm downstream at 220.5mm
# GDML comment indicates that this is 220.5mm, but the actual value is clearly 240.5mm!
ecal_front_z = 240.

side_Ecal_dx = 800.
side_Ecal_dy = 600.

# Dimensions of ECal parent volume
# The size is set to be 1mm larger than the thickness of the ECal calculated above
ecal_envelope_x = side_Ecal_dx
ecal_envelope_y = side_Ecal_dy
ecal_envelope_z = ECal_dz + 1

# Surround the ECal with scoring planes
# sp_ecal_front_z = ecal_front_z + (ecal_envelope_z - ECal_dz)/2 - sp_thickness/2 + clearance
sp_ecal_front_z = ecal_front_z - sp_thickness/2 - clearance      # v14
sp_ecal_back_z = ecal_front_z + ECal_dz + (ecal_envelope_z - ECal_dz)/2 + sp_thickness/2
sp_ecal_top_y = ECal_dy/2 + sp_thickness/2
sp_ecal_bot_y = -ECal_dy/2 - sp_thickness/2
sp_ecal_left_x = -ECal_dx/2 - sp_thickness/2
sp_ecal_right_x = ECal_dx/2 + sp_thickness/2
sp_ecal_mid_z = ecal_front_z + ECal_dz/2 + (ecal_envelope_z - ECal_dz)/2
radius68_thetalt10 = [  10.12233413, 9.921772, 11.38255086, 11.67991867, 13.14337347, 
                      13.17120624, 16.80994665, 17.83787244, 22.44684374, 23.74239886,
                      28.60564083, 30.27889678, 34.86404888, 36.39009394, 41.29309474,
                      43.34682279, 48.55982854, 50.80565589, 55.29496257, 57.92737879,
                      60.64828824, 65.51760517, 68.26709803, 76.32877518, 84.61219467,
                      103.3649691, 111.1692293, 119.2928089, 127.7357081, 136.4979268,
                      145.579465, 154.9803228, 164.7005, 174.7399968 ]
radius68_theta10to15 = [ 10.82307758, 11.17850518, 16.2185281, 18.62488713, 22.63408229, 
                        24.71769042, 30.11217538, 32.69939046, 37.99753196, 40.81619543,
                        45.89054775, 49.03066318, 54.00440948, 59.31733555, 63.40789682,
                        64.77580021, 73.00113678, 73.25561396, 78.8914776, 86.73962133,
                        97.05926327, 96.6932739, 111.6226151, 106.5960265, 109.477541,
                        144.2545942, 153.7581461, 163.5921179, 173.7565094, 184.2513208,
                        195.076552, 206.2322029, 217.7182737, 229.5347642 ]
radius68_theta15to20 = [ 12.79450901, 13.02698578, 21.27450933, 25.66008312, 31.78592103, 
                        35.99689874, 44.37101115, 48.82709363, 55.05972458, 59.68948687,
                        65.39866214, 70.59280337, 76.06007787, 82.22695257, 87.50371819,
                        90.60099831, 96.34848268, 101.4928478, 106.7157092, 105.0540604,
                        110.0653355, 148.3428736, 133.1449443, 146.997265, 173.3954389,
                        185.1307166, 196.1408667, 207.4772729, 219.1399351, 231.1288534,
                        243.4440277, 256.085458, 269.0531444, 282.3470868 ]
radius68_theta20to30 = [ 14.16989595, 15.4488322, 28.31044668, 37.54285657, 48.57288885, 
                        57.04243339, 68.99836079, 75.33388728, 85.00572867, 91.52574074,
                        102.5044698, 106.5315986, 116.2341378, 127.1121442, 133.8866375,
                        144.5121759, 162.1726963, 160.2986579, 171.386638, 182.5653112,
                        205.5853241, 196.3113071, 200.5907513, 228.7275694, 234.0298491,
                        253.7990618, 263.6872702, 273.5754785, 283.4636869, 293.3518953,
                        303.2401036, 313.128312, 323.0165203, 332.9047287 ]
radius68_theta30to60 = [ 22.50983127, 26.44537503, 58.24642887, 90.59076279, 130.0592014, 
                        157.4611392, 184.2187293, 202.6994588, 225.3488816, 243.3454167,
                        269.2456428, 280.6119298, 303.8591523, 322.0522722, 335.1780181,
                        350.3398234, 353.7763544, 373.9942362, 382.9453608, 401.9703438,
                        441.6281859, 432.5241826, 455.2878243, 492.2888656, 502.6653722,
                        519.9101788, 539.1604349, 558.410691, 577.6609471, 596.9112032,
                        616.1614593, 635.4117154, 654.6619715, 673.9122276 ]
def createAllRoc(labels, scores, positive_label,graphlabel,colors): #positive_label=1
    plt.title('RoC Analysis')
    for i in range(len(labels)):
        fpr, tpr, thresholds = metrics.roc_curve(labels[i], scores[i], pos_label=positive_label)
        print(thresholds)
        plt.plot(fpr, tpr, 'b', label=graphlabel[i],color = colors[i])
    plt.legend(loc='lower right')
    plt.plot([0,1],[0,1],'r--')
    plt.xlim([0,0.005])
    plt.ylim([0.5,1.01])
    #plt.xlim([-0.1,1.2])
    #plt.ylim([-0.1,1.2])
    plt.ylabel('True Positive Rate')
    plt.xlabel('False Positive Rate')
    plt.grid()
    plt.savefig('fid.png',dpi=400)
    plt.show()

def projection(pos_init,mom_init,z_final):
    if mom_init == [0,0,0]:
        print("Your input momentum is impossible")
    x_final = pos_init[0]+mom_init[0]/mom_init[2]*(z_final-pos_init[2])
    y_final = pos_init[1]+mom_init[1]/mom_init[2]*(z_final-pos_init[2])
    return [x_final,y_final,z_final]

def projectionlist(pos_init,mom_init,z_final):
    if mom_init == [0,0,0]:
        print("Your input momentum is impossible")
    x_final = []
    y_final = []
    for z in z_final:
        x = pos_init[0]+mom_init[0]/mom_init[2]*(z-pos_init[2])
        y = pos_init[1]+mom_init[1]/mom_init[2]*(z-pos_init[2])
        x_final+=[x]
        y_final+=[y]
    return [x_final,y_final,z_final]



#createAllRoc([y,y], [s1,scores], 1,['h1','h2'])

import uproot
zLayer = tagger_tracker_layerZs #recoil_tracker_layerZs #ecal_layerZs #np.arange(250,850,50)
file_path = '/Users/fdelzanno/Desktop/Incandela/Coding/ldmx-sw/10jobs1K/withTracking/8gev_1e_v14_kaon_ldmx-det-v14-8gev_run0_withTracking.root'
with uproot.open(file_path) as file:
    tree = file["LDMX_Events"]
    tree2 = file["LDMX_Events/TaggerTracks_TrackerReco"]

    
    keys = file["LDMX_Events/TaggerTracks_TrackerReco"].keys()
    print(f"Available keys in branch:")
    for key in keys:
        print(key)
    
    # pn
    '''
    event_numbers = tree["EventHeader/eventNumber_"].array(library='np')
    ts_pdgID = tree["TargetScoringPlaneHits_sim.pdgID_"].array(library='np')
    ts_px = tree["TargetScoringPlaneHits_sim.px_"].array(library='np')
    ts_py = tree["TargetScoringPlaneHits_sim.py_"].array(library='np')
    ts_pz = tree["TargetScoringPlaneHits_sim.pz_"].array(library='np')
    ts_x = tree["TargetScoringPlaneHits_sim.x_"].array(library='np')
    ts_y = tree["TargetScoringPlaneHits_sim.y_"].array(library='np')
    ts_z = tree["TargetScoringPlaneHits_sim.z_"].array(library='np')

    ecalsp_pdgID = tree["EcalScoringPlaneHits_sim.pdgID_"].array(library='np')
    ecalsp_px = tree["EcalScoringPlaneHits_sim.px_"].array(library='np')
    ecalsp_py = tree["EcalScoringPlaneHits_sim.py_"].array(library='np')
    ecalsp_pz = tree["EcalScoringPlaneHits_sim.pz_"].array(library='np')
    ecalsp_x = tree["EcalScoringPlaneHits_sim.x_"].array(library='np')
    ecalsp_y = tree["EcalScoringPlaneHits_sim.y_"].array(library='np')
    ecalsp_z = tree["EcalScoringPlaneHits_sim.z_"].array(library='np')
    
    ecalrecH_energy = tree["EcalRecHits_sim.energy_"].array(library='np')
    ecalrecH_isNoise = tree["EcalRecHits_sim.isNoise_"].array(library='np')
    ecalrecH_x = tree["EcalRecHits_sim.xpos_"].array(library='np')
    ecalrecH_y = tree["EcalRecHits_sim.ypos_"].array(library='np')
    ecalrecH_z = tree["EcalRecHits_sim.zpos_"].array(library='np')
    ecalrecH_id = tree["EcalRecHits_sim.id_"].array(library='np')
    ecalrecH_amp = tree["EcalRecHits_sim.amplitude_"].array(library='np')
    ecalrecH_time = tree["EcalRecHits_sim.time_"].array(library='np')
    '''
    #simulation
    event_numbers = tree["EventHeader/eventNumber_"].array(library='np')
    ts_pdgID = tree["TaggerSimHits_sim.pdgID_"].array(library='np')
    ts_px = tree["TaggerSimHits_sim.px_"].array(library='np')
    ts_py = tree["TaggerSimHits_sim.py_"].array(library='np')
    ts_pz = tree["TaggerSimHits_sim.pz_"].array(library='np')
    ts_x = tree["TaggerSimHits_sim.x_"].array(library='np')
    ts_y = tree["TaggerSimHits_sim.y_"].array(library='np')
    ts_z = tree["TaggerSimHits_sim.z_"].array(library='np')
    
    rs_pdgID = tree["RecoilSimHits_sim.pdgID_"].array(library='np')
    rs_px = tree["RecoilSimHits_sim.px_"].array(library='np')
    rs_py = tree["RecoilSimHits_sim.py_"].array(library='np')
    rs_pz = tree["RecoilSimHits_sim.pz_"].array(library='np')
    rs_x = tree["RecoilSimHits_sim.x_"].array(library='np')
    rs_y = tree["RecoilSimHits_sim.y_"].array(library='np')
    rs_z = tree["RecoilSimHits_sim.z_"].array(library='np')
    
    taggerTruth_pdgID = tree["TaggerTruthTracks_TrackerReco.pdgID_"].array(library='np')
    recoilTruth_pdgID = tree["RecoilTruthTracks_TrackerReco.pdgID_"].array(library='np')

    tagger_pdgID = tree["TaggerTracks_TrackerReco.pdgID_"].array(library='np')
    recoil_pdgID = tree["RecoilTracks_TrackerReco.pdgID_"].array(library='np')
    
    
    # Get the TBranchElement object
    state = tree["TaggerTracks_TrackerReco.trackStates_"]
    
    print(state)


#    tagger_trackStates = tree["TaggerTracks_TrackerReco.trackStates_"]
#
#    print(tagger_trackStates.GetLeaf("refX"))
    
#    ecalrecH_energy = tree["EcalRecHits_sim.energy_"].array(library='np')
#    ecalrecH_isNoise = tree["EcalRecHits_sim.isNoise_"].array(library='np')
#    ecalrecH_x = tree["EcalRecHits_sim.xpos_"].array(library='np')
#    ecalrecH_y = tree["EcalRecHits_sim.ypos_"].array(library='np')
#    ecalrecH_z = tree["EcalRecHits_sim.zpos_"].array(library='np')
#    ecalrecH_id = tree["EcalRecHits_sim.id_"].array(library='np')
#    ecalrecH_amp = tree["EcalRecHits_sim.amplitude_"].array(library='np')
#    ecalrecH_time = tree["EcalRecHits_sim.time_"].array(library='np')
    
    summed_det = tree['summedDet_'].array(library='np')
    
    #print(event_numbers)
    
    # Find the index of the event you want
    event_idx = np.where(event_numbers == 405)[0]
    print(event_idx)
    
    print("Number of electrons in Tagger Sim Hits: " + str(ts_pdgID[event_idx]))
    print("Number of electrons in Recoil Sim Hits: " + str(rs_pdgID[event_idx]))

    print("Number of electrons in Truth Tagger: " + str(taggerTruth_pdgID[event_idx]))
    print("Number of electrons in Truth Recoil: " + str(recoilTruth_pdgID[event_idx]))
    print("Number of electrons in Tagger: " + str(tagger_pdgID[event_idx]))
    print("Number of electrons in Recoil: " + str(recoil_pdgID[event_idx]))

    hitP_reco = []
    hitx_reco = []
    hity_reco = []
    hitz_reco = []
    
    hitP_tag = []
    hitx_tag = []
    hity_tag = []
    hitz_tag = []
    
    etraj = [0,0,0]
    gtraj = [0,0,0]
    gtrajProjected = [0,0,0]
    
    if len(event_idx) > 0:
        # if found that event
        event_idx = event_idx[0]
        print("summed_det",summed_det[event_idx])
        
        emomentum = [0,0,0]
        eposition = [0,0,0]
        etmomentum = [0,0,0]
        etposition = [0,0,0]
        gmomentum = [0,0,0]
        gposition = [0,0,0]
        maxPP = 0
        maxgP = 0
        for j in range(len(ts_x[event_idx])):
            thisP = [ts_px[event_idx][j],ts_py[event_idx][j],ts_pz[event_idx][j]]
            
            thisPosition = [ts_x[event_idx][j],ts_y[event_idx][j],ts_z[event_idx][j]]
            thispmag = thisP[0]**2+thisP[1]**2+thisP[2]**2

            if thispmag>maxPP and ts_pdgID[event_idx][j] ==abs(11) and thisP[2]>0: #and thispmag<3000**2:
                maxPP = thispmag
                etmomentum = thisP
                etposition = thisPosition
        maxP = 0
        maxgP = 0
        for j in range(len(rs_x[event_idx])):
            thisP = [rs_px[event_idx][j],rs_py[event_idx][j],rs_pz[event_idx][j]]
            
            thisPosition = [rs_x[event_idx][j],rs_y[event_idx][j],rs_z[event_idx][j]]
            thispmag = thisP[0]**2+thisP[1]**2+thisP[2]**2
            if thispmag>maxP and rs_pdgID[event_idx][j] ==abs(11) and thisP[2]>0: #abs(thisPosition[2]-sp_ecal_front_z)<0.5*sp_thickness:
                maxP = thispmag
                emomentum = thisP
                eposition = thisPosition
            if thispmag>maxgP and rs_pdgID[event_idx][j] ==22 and thisP[2]>0: #abs(thisPosition[2]-sp_ecal_front_z)<0.5*sp_thickness:
                maxgP = thispmag
                gmomentum = thisP
                gposition = thisPosition
        
        
        print('This event corresponds to ', event_numbers[event_idx])
        print('From Tagger SIM, eP = '+str(etmomentum[2])+'eTheta =' + str(180*np.arccos(etmomentum[2]/(maxPP**0.5))/np.pi))
        print(maxP)
        if not maxP == 0:
            etraj = projectionlist(eposition,emomentum,zLayer)
            #gtrajProjected = projectionlist(etposition,[-etmomentum[0],0-etmomentum[1],4000-etmomentum[2]] ,zLayer)
            theta = 180*np.arccos(emomentum[2]/(maxP**0.5))/np.pi
            
            print('The electron momentum is '+ str(emomentum)+' with angle '+str(theta)+' degrees')
        print(maxgP)
        if not maxgP ==0:
            gtraj = projectionlist(gposition,gmomentum,zLayer)
            print('Photon momentum: '+ str(gmomentum)+' with angle '+str(180*np.arccos(gmomentum[2]/(maxgP**0.5))/np.pi)+' degrees')
        #else:
            #eventNum =eventNum-1
        maxPPP = 0
        cP = [0,0,0]
        cPpdg = 1000
        for j in range(len(ts_x[event_idx])):
            thisP = [ts_px[event_idx][j],ts_py[event_idx][j],ts_pz[event_idx][j]]
            thispmag = thisP[0]**2+thisP[1]**2+thisP[2]**2
            if thispmag>maxPPP:
                maxPPP = thispmag
                cP = thisP
                cPpdg = ts_pdgID[event_idx][j]
        print([maxPPP**0.5,cP,cPpdg])
        
        
        #hitE = []
        #hitx = []
        #hity = []
        #hitz = []
        strange_hitE=[]
        strange_hitE_index=[]
        
        find_max_hitE = -1
        find_max_hitE_index = -1
        
#        for j in range(len(ecalrecH_z[event_idx])):
#            if not ecalrecH_isNoise[event_idx][j]:
#                hitz+=[ecalrecH_z[event_idx][j]]
#                hity+=[ecalrecH_y[event_idx][j]]
#                hitx+=[ecalrecH_x[event_idx][j]]
#                hitE+=[ecalrecH_energy[event_idx][j]]
#                # find hit with hitE>6000 and check
#                if ecalrecH_energy[event_idx][j]>6000:
#                    strange_hitE+=[ecalrecH_energy[event_idx][j]]
#                    strange_hitE_index.append(j)
#
#        maxHitZ = hitz[hitE.index(max(hitE))]
#        hitLayer = 0
#        while hitLayer <100 and abs(ecal_layerZs[hitLayer]-maxHitZ)>0.1:
#            hitLayer = hitLayer+1
#        print('the drawn event is '+ str(event_numbers[event_idx]))

    for j in range(len(ts_z[event_idx])):
        if (abs(ts_pdgID[event_idx][j]) == 11):
            hitz_tag+=[ts_z[event_idx][j]]
            hity_tag+=[ts_y[event_idx][j]]
            hitx_tag+=[ts_x[event_idx][j]]
            hitP_tag+=[((ts_pz[event_idx][j]**2)+(ts_px[event_idx][j]**2)+(ts_py[event_idx][j]**2))**0.5]

    print("HIT Y TAG")
    print(hity_tag)
    
    maxHitZ = hitz_tag[hitP_tag.index(max(hitP_tag))]
    hitLayer = 0
    while hitLayer <6 and abs(recoil_tracker_layerZs[hitLayer]-maxHitZ)>0.1:
        hitLayer = hitLayer+1
    
    for j in range(len(rs_z[event_idx])):
        if (abs(rs_pdgID[event_idx][j]) == 11):
            hitz_reco+=[rs_z[event_idx][j]]
            hity_reco+=[rs_y[event_idx][j]]
            hitx_reco+=[rs_x[event_idx][j]]
            hitP_reco+=[((rs_pz[event_idx][j]**2)+(rs_px[event_idx][j]**2)+(rs_py[event_idx][j]**2))**0.5]
            
    print("HIT Y RECO")
    print(hity_reco)

    maxHitZ = hitz_reco[hitP_reco.index(max(hitP_reco))]
    hitLayer = 0
    while hitLayer <6 and abs(recoil_tracker_layerZs[hitLayer]-maxHitZ)>0.1:
        hitLayer = hitLayer+1
    print('the drawn event is '+ str(event_numbers[event_idx]))
        
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d import Axes3D
    
    fig = plt.figure(figsize=(10, 7))
    ax = fig.add_subplot(111, projection='3d')
    
    # Setting labels
    ax.set_xlabel('Z-axis', fontweight='bold')
    ax.set_ylabel('X-axis', fontweight='bold')
    ax.set_zlabel('Y-axis', fontweight='bold')
    
    # Scatter plot for hits
    sctt = ax.scatter3D(hitz_tag+hitz_reco, hitx_tag+hitx_reco, hity_tag+hity_reco, c=(hitP_tag+hitP_reco), cmap='viridis')
    # Colorbar for energy deposition
    c = fig.colorbar(sctt, ax=ax, shrink=0.5, aspect=5, pad=0.1)
    #c.set_label('Energy Deposition (MeV)')
    c.set_label('Momentum (MeV)')
    
    # Plotting trajectories
    # Ensure etraj, gtraj, gtrajProjected are structured as [[x0, x1, ...], [y0, y1, ...], [z0, z1, ...]]
    #if etraj is not None and gtraj is not None and gtrajProjected is not None:
    #    l1, = ax.plot(etraj[2], etraj[0], etraj[1], label='electron trajectory')
        #l4, = ax.plot(gtraj[2], gtraj[0], gtraj[1], label='photon trajectory')
        #l5, = ax.plot(gtrajProjected[2], gtrajProjected[0], gtrajProjected[1], label='projected photon trajectory')
        
        # Adding legend
        #ax.legend(handles=[l1], loc='upper left')
        #ax.legend(handles=[l1, l4, l5], loc='upper left')
    
    #plt.savefig('/Users/fdelzanno/Desktop/Incandela/Coding/ldmx-sw/10jobs1K/withTracking/plots/event_display_1.png',dpi=900)
    plt.show()

    ''' for k in range(len(strange_hitE_index)):
            print("the",k,"th event")
            print("hitE greater than 6000MeV energy", strange_hitE[k])
            print("hitE greater than 6000MeV index", strange_hitE_index[k])
            print("x",ecalrecH_x[event_idx][strange_hitE_index[k]])
            print("y",ecalrecH_y[event_idx][strange_hitE_index[k]])
            print("z",ecalrecH_z[event_idx][strange_hitE_index[k]])
            print("amplitude",ecalrecH_amp[event_idx][strange_hitE_index[k]])
            print("id",ecalrecH_id[event_idx][strange_hitE_index[k]])
            print("isNoise",ecalrecH_isNoise[event_idx][strange_hitE_index[k]])
            print("time",ecalrecH_time[event_idx][strange_hitE_index[k]]) '''
    #current_energy = ecalrecH_energy[event_idx][j]
        #if current_energy > find_max_hitE:
            #    find_max_hitE = current_energy
                #    find_max_hitE_index = j
                #print("hit with max ecalRecHit_E",find_max_hitE_index)
        #print("ecalRecHit_E",find_max_hitE)
        #print("id",ecalrecH_id[event_idx][find_max_hitE_index])
        
        #eventNum = eventNum+1
    #x = np.array([5,7,8,7,2,17,2,9,4,11,12,9,6])
    #y = np.array([99,86,87,88,111,86,103,87,94,78,77,85,86])
    #colors = np.array([0, 10, 20, 30, 40, 45, 50, 55, 60, 70, 80, 90, 100])
    
    
'''
    if theta<15:
        if theta < 11:
            correctRoc = radius68_thetalt10
        else:
            correctRoc = radius68_theta10to15
    else:
        if theta>30:
            correctRoc = radius68_theta30to60
        else:
            if theta>20:
                correctRoc = radius68_theta20to30
            else:
                correctRoc = radius68_theta15to20
    r1 = correctRoc[hitLayer]
    #circlex1,circley1,circlez1 = drawcirc(eposition,emomentum,maxHitZ,r1)
    r2 = r1*2
    #circlex2,circley2,circlez2 = drawcirc(eposition,emomentum,maxHitZ,r2)
'''
