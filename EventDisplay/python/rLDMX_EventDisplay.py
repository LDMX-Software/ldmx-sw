from LDMX.Framework import EventTree
import numpy as np
import matplotlib.pyplot as plt
import sys
import ROOT as r
import random
from mpl_toolkits.mplot3d import Axes3D, art3d

def addBranch(tree, ldmx_class, branch_name):
    if tree == None:
        sys.exit('Set tree')

    if ldmx_class == 'EventHeader':
        branch = r.ldmx.EventHeader()
    elif ldmx_class == 'EcalVetoResult':
        branch = r.ldmx.EcalVetoResult()
    elif ldmx_class == 'HcalVetoResult':
        branch = r.ldmx.HcalVetoResult()
    elif ldmx_class == 'TriggerResult':
        branch = r.ldmx.TriggerResult()
    elif ldmx_class == 'SimParticle':
        branch = r.std.map(int, 'ldmx::'+ldmx_class)()
    else:
        branch = r.std.vector('ldmx::'+ldmx_class)()

    tree.SetBranchAddress(branch_name, r.AddressOf(branch))

    return branch
    
def trackPlotter(tree, event_number, tag, save, save_tag):
    
    recoilSimHits = addBranch(tree, 'SimTrackerHit', 'RecoilSimHits_{}'.format(tag))
    targetSP = addBranch(tree, 'SimTrackerHit', 'TargetScoringPlaneHits_{}'.format(tag))
    ecalRecHit = addBranch(tree, 'EcalHit', 'EcalRecHits_{}'.format(tag))
    digiRecoil = addBranch(tree, 'Measurement', 'DigiRecoilSimHits_{}'.format(tag))
    recoilTruth = addBranch(tree, 'StraightTrack', 'LinearRecoilTruthTracks_{}'.format(tag))
    recoil = addBranch(tree, 'StraightTrack', 'LinearRecoilTracks_{}'.format(tag))

    tree.GetEntry(event_number)
    
    recoilSim_x = []
    recoilSim_y = []
    recoilSim_z = []
    
    xpos_digi_tot = []
    ypos_digi_tot = []
    zpos_digi_tot = []
    
    first_sensor_z = []
    first_sensor_x = []
    first_sensor_y = []
    
    second_sensor_z = []
    second_sensor_x = []
    second_sensor_y = []

    ecal_end_x = []
    ecal_end_y = []
    ecal_end_z = []

    ecal_end_NOISE_x = []
    ecal_end_NOISE_y = []
    ecal_end_NOISE_z = []
    
    targetSP_x = []
    targetSP_y = []
    targetSP_z = []
    
    trackParams = []
    truthTrackParams = []

    for particle in recoilSimHits:
        recoilSim_z.append(particle.getPosition()[2])
        recoilSim_x.append(particle.getPosition()[0])
        recoilSim_y.append(particle.getPosition()[1])
        
    for sp_particle in targetSP:
        if (sp_particle.getPosition()[2] > 0):
            targetSP_z.append(sp_particle.getPosition()[2])
            targetSP_x.append(sp_particle.getPosition()[0])
            targetSP_y.append(sp_particle.getPosition()[1])
        
    for x_digi in digiRecoil:
        zpos_digi_tot.append(x_digi.getGlobalPosition()[0])
        xpos_digi_tot.append(x_digi.getGlobalPosition()[1])
        ypos_digi_tot.append(x_digi.getGlobalPosition()[2])
        
    for x_ecal in ecalRecHit:
        if (x_ecal.getZPos() < 250):
            if (x_ecal.isNoise()):
                ecal_end_NOISE_x.append(x_ecal.getXPos())
                ecal_end_NOISE_y.append(x_ecal.getYPos())
                ecal_end_NOISE_z.append(x_ecal.getZPos())
            else:
                ecal_end_x.append(x_ecal.getXPos())
                ecal_end_y.append(x_ecal.getYPos())
                ecal_end_z.append(x_ecal.getZPos())
            
    for x in recoil:
        trackParams.append((x.getSlopeX(), x.getInterceptX(), x.getSlopeY(), x.getInterceptY(), x.getDistanceToRecHit(), x.getChi2(), x.getTrackID()))
        first_sensor_z.append(x.getFirstSensorPosition()[0])
        first_sensor_x.append(x.getFirstSensorPosition()[1])
        first_sensor_y.append(x.getFirstSensorPosition()[2])
        
        second_sensor_z.append(x.getSecondSensorPosition()[0])
        second_sensor_x.append(x.getSecondSensorPosition()[1])
        second_sensor_y.append(x.getSecondSensorPosition()[2])
        
    for x_truth in recoilTruth:
        truthTrackParams.append((x_truth.getSlopeX(), x_truth.getInterceptX(), x_truth.getSlopeY(), x_truth.getInterceptY(), x_truth.getDistanceToRecHit(), x_truth.getChi2()))
    
    digiPoints = np.column_stack((zpos_digi_tot, xpos_digi_tot, ypos_digi_tot))
    ecalRecHits = np.column_stack((ecal_end_z, ecal_end_x, ecal_end_y))
    ecalRecHits_noise = np.column_stack((ecal_end_NOISE_z, ecal_end_NOISE_x, ecal_end_NOISE_y))
    recoilSim = np.column_stack((recoilSim_z, recoilSim_x, recoilSim_y))
    targetSP_hits = np.column_stack((targetSP_z, targetSP_x, targetSP_y))
    first_sensor_pos = np.column_stack((first_sensor_z, first_sensor_x, first_sensor_y))
    second_sensor_pos = np.column_stack((second_sensor_z, second_sensor_x, second_sensor_y))
    
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')

    x_sensor_uncertainty = 0.006 #mm
    y_sensor_uncertainty = 0.085 # mm
    
    for zi, xi, yi in zip(first_sensor_pos[:,0], first_sensor_pos[:,1], first_sensor_pos[:,2]):
        ax.plot([zi, zi], [xi - x_sensor_uncertainty, xi + x_sensor_uncertainty], [yi, yi], color='black',linewidth=2)  # x error
        ax.plot([zi, zi], [xi, xi], [yi - y_sensor_uncertainty, yi + y_sensor_uncertainty], color='black',linewidth=2)  # y error

    for zi, xi, yi in zip(second_sensor_pos[:,0], second_sensor_pos[:,1], second_sensor_pos[:,2]):
        ax.plot([zi, zi], [xi - x_sensor_uncertainty, xi + x_sensor_uncertainty], [yi, yi], color='black',linewidth=2)  # x error
        ax.plot([zi, zi], [xi, xi], [yi - y_sensor_uncertainty, yi + y_sensor_uncertainty], color='black',linewidth=2)  # y error
        
    ax.scatter(digiPoints[:,0], digiPoints[:,1], digiPoints[:,2], c='b', marker='o', label='DigiRecoil SimHit', s=25)
    ax.scatter(ecalRecHits[:, 0], ecalRecHits[:, 1], ecalRecHits[:, 2], c='purple', label='ECalRecHit', s=50, alpha=0.5)
    ax.scatter(recoilSim[:, 0], recoilSim[:, 1], recoilSim[:, 2], marker='o', c='gray', label='RecoilSimHits', s=100, alpha=0.25)
    ax.scatter(targetSP_hits[:, 0], targetSP_hits[:, 1], targetSP_hits[:, 2], marker='o', c='yellow', label='Target SP Hits', s=50, alpha=0.5)
    ax.scatter(first_sensor_pos[:,0], first_sensor_pos[:,1], first_sensor_pos[:,2], marker='o', c='red', label='First Sensor Point', s=30, alpha=0.75)
    ax.scatter(second_sensor_pos[:,0], second_sensor_pos[:,1], second_sensor_pos[:,2], marker='o', c='orange', label='Second Sensor Point', s=30, alpha=0.75)
    
    if (len(ecal_end_NOISE_z) > 0):
        ax.scatter(ecalRecHits_noise[:, 0], ecalRecHits_noise[:, 1], ecalRecHits_noise[:, 2], c='magenta', label='NOISE ECalRecHit', s=50, alpha=0.5)

    for track in trackParams:
        if (track[6] == 1):
            ax.plot([0.0, ecalRecHits[0][0]], [track[1], track[0]*ecalRecHits[0][0]+track[1]], [track[3], track[2]*ecalRecHits[0][0]+track[3]], 'b--', label=f'TrackID = {track[6]}, d_RecHit = {track[4]:.2f} mm, chi2 = {track[5]:.3f}')
        else:
            ax.plot([0.0, ecalRecHits[0][0]], [track[1], track[0]*ecalRecHits[0][0]+track[1]], [track[3], track[2]*ecalRecHits[0][0]+track[3]], 'r--', label=f'TrackID = {track[6]}, d_RecHit = {track[4]:.2f} mm, chi2 = {track[5]:.3f}')
            
    if (len(truthTrackParams) > 0):
        for truthTrack in truthTrackParams:
            ax.plot([0.0, ecalRecHits[0][0]], [truthTrack[1], truthTrack[0]*ecalRecHits[0][0]+truthTrack[1]], [truthTrack[3], truthTrack[2]*ecalRecHits[0][0]+truthTrack[3]], color='blue', label=f'Truth Track, d_RecHit = {truthTrack[4]:.2f} mm, chi2 = {truthTrack[5]:.3f}')
    
    x = ecalRecHits[0][0]
    y_range = np.linspace(min(ecalRecHits[:,1]) - 5, max(ecalRecHits[:,1]) + 5, 50)
    z_range = np.linspace(min(ecalRecHits[:,2]) - 5, max(ecalRecHits[:,2]) + 5, 50)
    Y, Z = np.meshgrid(y_range, z_range)
    X = np.full(Y.shape, ecalRecHits[0][0])
        
    plane = ax.plot_surface(X, Y, Z, color='red', alpha=0.1, edgecolor='none')
        
    for recHit in ecalRecHits:
        center_z, center_x, center_y = recHit[0], recHit[1], recHit[2]
        radius = 3.87
        theta = np.linspace(0, 2 * np.pi, 100)
        circle_x = center_x + radius * np.cos(theta)
        circle_y = center_y + radius * np.sin(theta)
        circle_z = np.full_like(circle_x, center_z)
        ax.plot(circle_z, circle_x, circle_y, c='purple')

    # Labels and title
    ax.set_xlabel('z [mm]')
    ax.set_ylabel('x [mm]')
    ax.set_zlabel('y [mm]')

    fig.text(0.15, 0.9, 'LDMX', ha='center', fontsize=36, fontweight='bold')
    fig.text(0.43, 0.9, 'Work in Progress', ha='center', fontsize=24, fontstyle='italic')
    fig.text(0.85, 0.9, '(4 GeV)', ha='center', fontsize=12, fontstyle='italic')
    fig.subplots_adjust(right=0.99)  # Adjust the value as needed
    left, bottom, width, height = [0.06, 0.05, 0.85, 0.85]  # Adjust these values as needed
    ax.set_position([left, bottom, width, height])

    ax.legend(fontsize=10)
    
    plt.savefig(f'{save}/{tag}_eventDisplay_eventID_{event_number}_{save_tag}.png')

#    UNCOMMENT THESE LINES IF YOU WANT A ZOOMED IN PLOT (i.e. just recoil points)
#    ax.set_xlim(0, 50)
#
#    x_low_lim = -6
#    x_up_lim = -4
#    y_low_lim = -18.5
#    y_up_lim = -17
#
#    x_target = 0.0
#    y_target_range = np.linspace(x_low_lim, x_up_lim, 50)
#    z_target_range = np.linspace(y_low_lim, y_up_lim, 50)
#    Y_target, Z_target = np.meshgrid(y_target_range, z_target_range)
#    X_target = np.full(Y_target.shape, 0.0)
#    target_plane = ax.plot_surface(X_target, Y_target, Z_target, color='purple', alpha=0.1, edgecolor='none')
#
#    ax.set_ylim(x_low_lim, x_up_lim)
#    ax.set_zlim(y_low_lim, y_up_lim)
#    plane.set_visible(False)
#
#    plt.savefig(f'{save}/{tag}_eventDisplay_eventID_{event_number}_ZOOM.png')
    
def main():
    tree = r.TChain("LDMX_Events")
    tree.Add('events_5000_rLDMX_v1_yZERO_new3Dpoints.root')

    nentries = tree.GetEntries()
    print("nentries = ", nentries)
    
    tag = 'rLDMX_v1'
    save_loc = 'plots/event_displays/'
    event_number = 26
    save_tag = 'yZERO_new3Dpoints'

#    random_numbers = random.sample(range(0, 101), 10)
#    for number in random_numbers:
    trackPlotter(tree, event_number, tag, save_loc, save_tag)

if __name__ == "__main__":
    main()

