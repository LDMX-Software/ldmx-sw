#include <stdio.h>
#include <iostream>
#include "TrigScint/objdef.h"
#include "TrigScint/trackproducer.h"


void trackproducer_hw(Cluster Pad1[NTRK],Cluster Pad2[NCLUS],Cluster Pad3[NCLUS],Track outTrk[NTRK],ap_int<12> lookup[NCENT][COMBO][2]){
	#pragma HLS ARRAY_PARTITION variable=Pad1 dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=Pad2 dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=Pad3 dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=outTrk dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=lookup dim=0 complete
	#pragma HLS PIPELINE II=10
	Track test;
	#pragma HLS ARRAY_PARTITION variable=test complete

	for(int i = 0;i<NTRK;i++){
		if(2*Pad1[i].Seed.bID>2*NCHAN){continue;}
		for(int I = 0;I<COMBO; I++){
			clearTrack(test);
			if(not(Pad1[i].Seed.Amp>0)){continue;}//Continue if Seed not Satisfied
			ap_int<12> centroid = 2*Pad1[i].Seed.bID;
			if(Pad1[i].Sec.Amp>0){
				centroid+=1;
			}
			cpyCluster(test.Pad1,Pad1[i]);
			if((lookup[centroid][I][0]==-1)or(lookup[centroid][I][1]==-1)){continue;}//Pattern Empty
			if(not(Pad2[lookup[centroid][I][0]/4].Seed.Amp>0)){continue;}//Continue if Seed not Satisfied
			if((lookup[centroid][I][0]%4==0)and((Pad2[lookup[centroid][I][0]/4].Sec.bID>=0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==1))){continue;}//Continue if Sec is not Expected, and not Empty
			if((lookup[centroid][I][0]%4==1)and((Pad2[lookup[centroid][I][0]/4].Sec.bID<0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==1))){continue;}//Continue if Sec is Expected, and Empty
			if((lookup[centroid][I][0]%4==2)and((Pad2[lookup[centroid][I][0]/4].Sec.bID>=0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==0))){continue;}//Continue if Sec is not Expected, and not Empty
			if((lookup[centroid][I][0]%4==3)and((Pad2[lookup[centroid][I][0]/4].Sec.bID<0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==0))){continue;}//Continue if Sec is Expected, and Empty
			if(not(Pad3[lookup[centroid][I][1]/4].Seed.Amp>0)){continue;}//Continue if Seed not Satisfied
			if((lookup[centroid][I][1]%4==0)and((Pad3[lookup[centroid][I][1]/4].Sec.bID>=0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==1))){continue;}//Continue if Sec is not Expected, and not Empty
			if((lookup[centroid][I][1]%4==1)and((Pad3[lookup[centroid][I][1]/4].Sec.bID<0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==1))){continue;}//Continue if Sec is Expected, and Empty
			if((lookup[centroid][I][1]%4==2)and((Pad3[lookup[centroid][I][1]/4].Sec.bID>=0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==0))){continue;}//Continue if Sec is not Expected, and not Empty
			if((lookup[centroid][I][1]%4==3)and((Pad3[lookup[centroid][I][1]/4].Sec.bID<0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==0))){continue;}//Continue if Sec is Expected, and Empty
			cpyCluster(test.Pad2,Pad2[lookup[centroid][I][0]/4]);
			cpyCluster(test.Pad3,Pad3[lookup[centroid][I][1]/4]);
			calcResid(test);
			if(test.resid<outTrk[i].resid){
				cpyTrack(outTrk[i],test);
			}
		}
	}
	for(int i = 1;i<NTRK-1;i++){
		if((outTrk[i-1].Pad2.Seed.bID==outTrk[i].Pad2.Seed.bID)and(outTrk[i].Pad2.Seed.bID>=0)){
			if(outTrk[i-1].resid<=outTrk[i].resid){clearTrack(outTrk[i]);}else{clearTrack(outTrk[i-1]);}
		}
		if((outTrk[i].Pad2.Seed.bID==outTrk[i+1].Pad2.Seed.bID)and(outTrk[i+1].Pad2.Seed.bID>=0)){
			if(outTrk[i+1].resid<=outTrk[i].resid){clearTrack(outTrk[i]);}else{clearTrack(outTrk[i+1]);}
		}
		if((outTrk[i-1].Pad2.Seed.bID==outTrk[i+1].Pad2.Seed.bID)and(outTrk[i+1].Pad2.Seed.bID>=0)){
			if(outTrk[i-1].resid<=outTrk[i+1].resid){clearTrack(outTrk[i+1]);}else{clearTrack(outTrk[i-1]);}
		}
	}
	//GETS RID OF SHARED CLUSTERS IN THE THIRD PAD
	for(int i = 1;i<NTRK-1;i++){
		//Here we check triplets and remove if necessary
		if((outTrk[i-1].Pad3.Seed.bID==outTrk[i].Pad3.Seed.bID)and(outTrk[i].Pad3.Seed.bID>=0)){
			if(outTrk[i-1].resid<=outTrk[i].resid){clearTrack(outTrk[i]);}else{clearTrack(outTrk[i-1]);}
		}
		if((outTrk[i].Pad3.Seed.bID==outTrk[i+1].Pad3.Seed.bID)and(outTrk[i+1].Pad3.Seed.bID>=0)){
			if(outTrk[i+1].resid<=outTrk[i].resid){clearTrack(outTrk[i]);}else{clearTrack(outTrk[i+1]);}
		}
		if((outTrk[i-1].Pad3.Seed.bID==outTrk[i+1].Pad3.Seed.bID)and(outTrk[i+1].Pad3.Seed.bID>=0)){
			if(outTrk[i-1].resid<=outTrk[i+1].resid){clearTrack(outTrk[i+1]);}else{clearTrack(outTrk[i-1]);}
		}
	}
	return;
}
