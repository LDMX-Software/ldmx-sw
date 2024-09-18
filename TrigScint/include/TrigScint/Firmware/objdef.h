#ifndef OBJDEF_H
#define OBJDEF_H

#include "ap_int.h"
#define NTIMES 6
#define NHITS 25
#define NCLUS 25
#define NCHAN 50
#define NTRK 10
#define W = 10

#define NCENT 99

#define NDIGIS 14
#define COMBO 9

//2*NCHAN*NTIMES are the number of bytes per event plus 4+4+4+3+1 bytes for the header

#define NSAMPLES 6

//NSAMPLES/8 is the number of 64 bit words

#define NWORDS 72

struct Digi {
	int mID, bID;
	int adc0, adc1, adc2, adc3, adc4, adc5;
	int tdc0, tdc1, tdc2, tdc3, tdc4, tdc5;
};
inline void clearDigi(Digi & c){
	c.mID=0;c.bID=0;
	c.adc0=0;c.adc1=0;c.adc2=0;c.adc3=0;c.adc4=0;c.adc5=0;
	c.tdc0=0;c.tdc1=0;c.tdc2=0;c.tdc3=0;c.tdc4=0;c.tdc5=0;
}
struct Hit {
	ap_int<12> mID, bID;
	ap_int<12> Amp, Time; //TrigTime;
};
inline void clearHit(Hit & c){
	c.mID=0; c.bID=-1; c.Amp=0; c.Time=0; //c.TrigTime=0.0;
}
inline void cpyHit(Hit & c1, Hit & c2){
	c1.mID=c2.mID;c1.bID=c2.bID;c1.Amp=c2.Amp;c1.Time=c2.Time;
}

struct Cluster {
	Hit Seed; Hit Sec;
	ap_int<11> Cent;
	//int nhits, mID, SeedID;
	//float CentX, CentY, CentZ, Amp, Time, TrigTime;
};
inline void clearClus(Cluster & c){
	clearHit(c.Seed);clearHit(c.Sec);c.Cent = (ap_int<11>)(0);//clearHit(c.For);
}
inline void calcCent(Cluster & c){
	if(c.Seed.Amp>0){
		c.Cent = (ap_int<12>)(10.*((float)(c.Seed.Amp*c.Seed.bID+c.Sec.Amp*c.Sec.bID))/((float)(c.Seed.Amp+c.Sec.Amp)));
	}else{
		c.Cent=(ap_int<12>)(0);
	}
}
inline void cpyCluster(Cluster & c1, Cluster & c2){
	cpyHit(c1.Seed,c2.Seed);cpyHit(c1.Sec,c2.Sec);
}

struct Track {
	Cluster Pad1; Cluster Pad2; Cluster Pad3;
	ap_int<12> resid;
};
inline void clearTrack(Track & c){
	clearClus(c.Pad1);clearClus(c.Pad2);clearClus(c.Pad3);
	c.resid=5000;
}
inline ap_int<12> calcTCent(Track & c){
	calcCent(c.Pad1);calcCent(c.Pad2);calcCent(c.Pad3);
	float one = (float)c.Pad1.Cent;
	float two = (float)c.Pad2.Cent;
	float three = (float)c.Pad3.Cent;
	float mean = (one+two+three)/3.0;
	ap_int<12> Cent = (ap_int<10>)((int)(mean));
	return Cent;
}
inline void calcResid(Track & c){
	calcCent(c.Pad1);calcCent(c.Pad2);calcCent(c.Pad3);
	float one = (float)c.Pad1.Cent;
	float two = (float)c.Pad2.Cent;
	float three = (float)c.Pad3.Cent;
	float mean = (one+two+three)/3.0;
	c.resid = (ap_int<12>)((int)(((one-mean)*(one-mean)+(two-mean)*(two-mean)+(three-mean)*(three-mean))/3.0));
}
inline void cpyTrack(Track & c1, Track & c2){
	cpyCluster(c1.Pad1,c2.Pad1);cpyCluster(c1.Pad2,c2.Pad2);cpyCluster(c1.Pad3,c2.Pad3);
	c1.resid=c2.resid;
}

#endif
