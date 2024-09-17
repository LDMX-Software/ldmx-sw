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
	std::cout<<"HELLO 2 HERE 1"<<std::endl;
	Track test;
	#pragma HLS ARRAY_PARTITION variable=test complete

	for(int i = 0;i<NTRK;i++){
		//TRYING FOR SHITS AND GRINS
		//if(Pad1[i].Sec.Amp>Pad1[i].Seed.Amp){
		//  Pad1[i].Seed.bID=Pad1[i].Sec.bID;
		//}
		std::cout<<"Pad1 Amp and 2 bID: "<<Pad1[i].Seed.Amp<<" "<<2*Pad1[i].Seed.bID<<std::endl;
		if(2*Pad1[i].Seed.bID>100){continue;}
		for(int I = 0;I<COMBO; I++){
			clearTrack(test);
			//std::cout<<"GOT HERE"<<std::endl;
			if(not(Pad1[i].Seed.Amp>0)){continue;}//Continue if Seed not Satisfied
			ap_int<12> centroid = 2*Pad1[i].Seed.bID;
			if(Pad1[i].Sec.Amp>0){
				centroid+=1;
			}
			std::cout<<"Pad2 Amp and 2 bID: "<<Pad2[lookup[centroid][I][0]/4].Seed.Amp<<" "<<2*Pad2[lookup[centroid][I][0]/4].Seed.bID<<std::endl;
			std::cout<<"Pad3 Amp and 2 bID: "<<Pad3[lookup[centroid][I][0]/4].Seed.Amp<<" "<<2*Pad3[lookup[centroid][I][0]/4].Seed.bID<<std::endl;
			std::cout<<"HELLO 2 HERE 2.1"<<std::endl;
			std::cout<<"HELLO 2 HERE 2.1"<<std::endl;
			std::cout<<"HELLO 2 HERE 2.1"<<std::endl;
			cpyCluster(test.Pad1,Pad1[i]);
			std::cout<<"HELLO 2 HERE 2.12"<<std::endl;
			if((lookup[centroid][I][0]==-1)or(lookup[centroid][I][1]==-1)){continue;}//Pattern Empty
			std::cout<<"HELLO 2 HERE 2.13"<<std::endl;
			if(not(Pad2[lookup[centroid][I][0]/4].Seed.Amp>0)){continue;}//Continue if Seed not Satisfied
			std::cout<<"HELLO 2 HERE 2.14"<<std::endl;
			if((lookup[centroid][I][0]%4==0)and((Pad2[lookup[centroid][I][0]/4].Sec.bID>=0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==1))){continue;}//Continue if Sec is not Expected, and not Empty
			std::cout<<"HELLO 2 HERE 2.15"<<std::endl;
			if((lookup[centroid][I][0]%4==1)and((Pad2[lookup[centroid][I][0]/4].Sec.bID<0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==1))){continue;}//Continue if Sec is Expected, and Empty
			std::cout<<"HELLO 2 HERE 2.16"<<std::endl;
			if((lookup[centroid][I][0]%4==2)and((Pad2[lookup[centroid][I][0]/4].Sec.bID>=0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==0))){continue;}//Continue if Sec is not Expected, and not Empty
			std::cout<<"HELLO 2 HERE 2.17"<<std::endl;
			if((lookup[centroid][I][0]%4==3)and((Pad2[lookup[centroid][I][0]/4].Sec.bID<0)or(Pad2[lookup[centroid][I][0]/4].Seed.bID%2==0))){continue;}//Continue if Sec is Expected, and Empty
			std::cout<<"HELLO 2 HERE 2.18"<<std::endl;
			if(not(Pad3[lookup[centroid][I][1]/4].Seed.Amp>0)){continue;}//Continue if Seed not Satisfied
			std::cout<<"HELLO 2 HERE 2.2"<<std::endl;
			if((lookup[centroid][I][1]%4==0)and((Pad3[lookup[centroid][I][1]/4].Sec.bID>=0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==1))){continue;}//Continue if Sec is not Expected, and not Empty
			std::cout<<"HELLO 2 HERE 2.21"<<std::endl;
			if((lookup[centroid][I][1]%4==1)and((Pad3[lookup[centroid][I][1]/4].Sec.bID<0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==1))){continue;}//Continue if Sec is Expected, and Empty
			std::cout<<"HELLO 2 HERE 2.22"<<std::endl;
			if((lookup[centroid][I][1]%4==2)and((Pad3[lookup[centroid][I][1]/4].Sec.bID>=0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==0))){continue;}//Continue if Sec is not Expected, and not Empty
			std::cout<<"HELLO 2 HERE 2.23"<<std::endl;
			if((lookup[centroid][I][1]%4==3)and((Pad3[lookup[centroid][I][1]/4].Sec.bID<0)or(Pad3[lookup[centroid][I][1]/4].Seed.bID%2==0))){continue;}//Continue if Sec is Expected, and Empty
			std::cout<<"HELLO 2 HERE 2.24"<<std::endl;
			cpyCluster(test.Pad2,Pad2[lookup[centroid][I][0]/4]);
			cpyCluster(test.Pad3,Pad3[lookup[centroid][I][1]/4]);
			calcResid(test);
			std::cout<<"HELLO 2 HERE 2.3"<<std::endl;
			if(test.resid<outTrk[i].resid){
				cpyTrack(outTrk[i],test);
			}
			std::cout<<"HELLO 2 HERE 2.4"<<std::endl;
		}
	}
	std::cout<<"HELLO 2 HERE 3"<<std::endl;
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
