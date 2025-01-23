from LDMX.Framework import EventTree
import numpy as np
import matplotlib.pyplot as plt
import uproot
import math
import time
import copy
import os
import sys
import ROOT as r
import statistics as stat
from mpl_toolkits.mplot3d import Axes3D, art3d
import matplotlib.animation as animation
from sklearn.linear_model import LinearRegression

def pause():
    programPause = input("Press the <ENTER> key to continue...")
    
def trackPlotter(tree, event_number, tag, save):
    recoilSimHits = addBranch(tree, 'SimTrackerHit', 'RecoilSimHits_{}'.format(tag))
    ecalRecHit = addBranch(tree, 'EcalHit', 'EcalRecHits_{}'.format(tag))
    digiRecoil = addBranch(tree, 'Measurement', 'DigiRecoilSimHits_{}'.format(tag))
    recoilTruth = addBranch(tree, 'StraightTrack', 'RecoilTruthTracks_{}'.format(tag))
    recoil = addBranch(tree, 'StraightTrack', 'LinearRecoilTracks_{}'.format(tag))

    tree.GetEntry(event_number)
    
    recoilSim_x = []
    recoilSim_y = []
    recoilSim_z = []
    
    xpos_digi_tot = []
    ypos_digi_tot = []
    zpos_digi_tot = []
    
    ecal_end_x = []
    ecal_end_y = []
    ecal_end_z = []
    
    trackParams = []
    truthTrackParams = []
    
    for particle in recoilSimHits:
        recoilSim_z.append(particle.getPosition()[2])
        recoilSim_x.append(particle.getPosition()[0])
        recoilSim_y.append(particle.getPosition()[1])
        
    for x_digi in digiRecoil:
        zpos_digi_tot.append(x_digi.getGlobalPosition()[0])
        xpos_digi_tot.append(x_digi.getGlobalPosition()[1])
        ypos_digi_tot.append(x_digi.getGlobalPosition()[2])
        
    for x_ecal in ecalRecHit:
        if (x_ecal.getZPos() < 250):
            ecal_end_x.append(x_ecal.getXPos())
            ecal_end_y.append(x_ecal.getYPos())
            ecal_end_z.append(x_ecal.getZPos())
            
    for x in recoil:
        trackParams.append((x.getSlopeX(), x.getInterceptX(), x.getSlopeY(), x.getInterceptY(), x.getDistanceToRecHit()))
        
    for x_truth in recoilTruth:
        truthTrackParams.append((x_truth.getSlopeX(), x_truth.getInterceptX(), x_truth.getSlopeY(), x_truth.getInterceptY(), x_truth.getDistanceToRecHit()))
    
        
    digiPoints = np.column_stack((zpos_digi_tot, xpos_digi_tot, ypos_digi_tot))
    ecalRecHits = np.column_stack((ecal_end_z, ecal_end_x, ecal_end_y))
    recoilSim = np.column_stack((recoilSim_z, recoilSim_x, recoilSim_y))
    
    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')

    x_unc = 0.006 #mm
    y_unc = 0.12 # mm

    for zi, xi, yi in zip(digiPoints[:,0], digiPoints[:,1], digiPoints[:,2]):
        ax.plot([zi, zi], [xi - x_unc, xi + x_unc], [yi, yi], color='black',linewidth=2)  # x error
        ax.plot([zi, zi], [xi, xi], [yi - y_unc, yi + y_unc], color='black',linewidth=2)  # y error

    ax.scatter(digiPoints[:,0], digiPoints[:,1], digiPoints[:,2], c='b', marker='o', label='DigiRecoil SimHit', s=25)
    ax.scatter(ecalRecHits[:, 0], ecalRecHits[:, 1], ecalRecHits[:, 2], c='purple', label='ECalRecHit', s=50, alpha=0.5)
    ax.scatter(recoilSim[:, 0], recoilSim[:, 1], recoilSim[:, 2], marker='o', c='gray', label='RecoilSimHits', s=50, alpha=0.5)

    i=0
    for track in trackParams:
        ax.plot([0.0, ecalRecHits[0][0]], [track[1], track[0]*ecalRecHits[0][0]+track[1]], [track[3], track[2]*ecalRecHits[0][0]+track[3]], 'r--', label=f'Reconstructed Track {i+1}, d_RecHit = {track[4]:.2f} mm')
        i += 1
    
    for truthTrack in truthTrackParams:
        ax.plot([0.0, ecalRecHits[0][0]], [truthTrack[1], truthTrack[0]*ecalRecHits[0][0]+truthTrack[1]], [truthTrack[3], truthTrack[2]*ecalRecHits[0][0]+truthTrack[3]], color='blue', label=f'Truth Track, d_RecHit = {truthTrack[4]:.2f} mm')

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
    
    plt.savefig(f'{save}/{tag}_eventDisplay_eventID_{event_number}.png')

    ax.set_xlim(0, 50)
    ax.set_ylim(-10, 10)
    ax.set_zlim(-10, 10)
    plane.set_visible(False)

    plt.savefig(f'{save}/{tag}_eventDisplay_eventID_{event_number}_ZOOM.png')

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
    
def main():
    tree = r.TChain("LDMX_Events")
    tree.Add('/Users/fdelzanno/Desktop/Incandela/Coding/ldmx-sw/events_100_rLDMXV2.root')

    nentries = tree.GetEntries()
    print("nentries = ", nentries)
    
    tag = 'rLDMX_v2'
    save_loc = '/Users/fdelzanno/Desktop/Incandela/Coding/ldmx-sw'
    
    trackPlotter(tree, 1, tag, save_loc)
    

if __name__ == "__main__":
    main()

