#include "Tracking/Digitization/CDFSiSensorSim.h"

/*

//TODO:: use ldmx logger instead
std::map<ChargeCarrier,SiElectrodeDataCollection> computeElectrodeData() {

  if (debug_) {

    //std::cout<<__PRETTY_FUNCTION__<<" sensor " <<std::endl;
    //std::cout<<" Sense Strips: "
  }
  
  depositChargeOneSense();
  transferChargeToReadout();
  
  return readout_data_;
    
  
}


//Clear readout data

void clearReadout() {

  for (auto pair : readout_data_) {

    readout_data_[pair.first].clear();
  }  
}

//Clear sense data
void clearSense() {

  for (auto pair : sense_dat_) {
    sense_data[pair.first].clear();
  }
  
}

//TODO Implement
void lorentzCorrect(const Acts::Vector3& position, const ChargeCarrier& carrier) {
    
}

//TODO:: Implement
void depositChargeOnSense() {
  //if (debug_)
  //  std::cout<<__PRETTY_FUNCTION__<<" depositChargeOnSense for sensor " << std:endl;
  
  //for (auto& carrier : tracking::digitization::ChargeCarriers) {
  
  
  //if (sensor_.hasElectrodesOnSide(carrier)) {
  
  //Use local origin to compute the drift direction
  //drift_direction_[carrier] = driftDirection(carrier, Acts::Vector3(0.,0.,0.));
  // }
  //}
  
  //Acts::Transform3 global_to_sensor     = sensor_.getGeometry().getGlobalToLocal();
  //std::vector<ldmx::SimTrackerHit> hits = sensor_.getReadout().getHits();

  for (auto& hit : hits) {

    Acts::Vector3 hitPosition(hit.getPosition()[0],hit.getPosition()[1],hit.getPosition()[2]);
    
    
    if (debug_) {
      std::cout<<__PRETTY_FUNCTION__<<"Hit Point: ["<<hitPosition(0)<<","<<hitPosition(1)<<","<<hitPosition(2)<<std::endl;
      
    }

    TrackSegment track_segment = new TrackSegment(hit);
    track_segment.transform(global_to_sensor);

    int nsegments = 0;

    for (auto& carrier : tracking::digitization::carriers) {

      if (sensor_.hasElectrodeOnSide(carrier))
        nsegments = std::max(nsegments, nSegments(track_segment, carrier, deposition_granularity));
      
    }

    double segment_length = track_segment.getLength() /  nsegments;
    double segment_charge = (track_segment.getEloss() / nsegment ) / sensor_.getBulk().ENERGY_EHPAIR;

    Acts::Vector3 segment_step   = segment_length * track_segment.getDirection();
    Acts::Vector3 segment_center = track_segment.getP1() + (0.5 * segment_step);

    if (debug_) {
      std::cout<<"segment length " << segment_length << std::endl;
      std::cout<<"segment charge " << segment_charge << std::endl;
      std::cout<<"segment step   " << segment_step << std::endl;
    }

    for ( int iseg = 0 ; iseg < nsegments; iseg++) {
      if (debug_)
        std::cout<<"segment "<< iseg<<" segment center"<<std::endl;

      for (auto& carrier : tracking::digitization::carriers) {

        if (sensor_.hasElectrodesOnSide(carrier)) {

          SiSensorElectrodes electrodes = sensor_.getSenseElectrodes(carrier);

          //Apply collection inefficiency for charge trapping: require between 0 and 1

          double collection_efficiency = 1.0 - 10 * trapping_*
                                         driftVector(segment_center,carrier).norm();

          collection_efficiency = std::max(0.0, std::min(1.0,collection_efficiency));

          segment_charge *= collection_efficiency;
          
          ChargeDistribution charge_distribution = diffusionDistribution(segment_charge, segment_center, carrier);
          charge_distribution.trasnform(electrodes.getParentToLocal());

          std::map<int,int> sense_charge = electrodes.computeElectrodeData(charge_distribution);

          sense_data[carrier].add(SiElectrodeDataCollection(sense_charge,hit));
          
        } // has electrodes
      }//loop on carriers
    }// loop on segments
  }//loop on hits
}//deposit charge on sense


//Calculate charge distribution for carrier
ChargeDistribution diffusionDistribution(double segment_charge,
                                         const Acts::Vector3& origin,
                                         const ChargeCarrier& carrier) {

  if (debug_)
    std::cout<<__PRETTY_FUNCTION__<<" calculating charge distribution for carrier"<<std::endl;

  double distance  = sensor_.distanceFromSide(origin, carrier);
  double thickness = sensor_.getThickness();

  if (distance < -distance_error_threshold || distance > distance_error_threshold) {
    std::cout<<"ERROR::"<<__PRETTY_FUNCTION__<<" Distance is outside of sensor by more than "<<distance_error_threshold<<std::endl; 
  }

  else if (distance < 0.)
    distance = 0.;
  else if (distance > thickness)
    distance = thickness;


  double bias_voltage      = sensor_.getBiasVoltage();
  double depletion_voltage = sensor_.getDepletionVoltage();

  //Common factors

  double deltaV = bias_voltage - depletionVoltage;
  double sumV   = bias_voltage + deplettionVoltage;
  double common_factor = 2.0 * distance * depletion_voltage / thickness;

  
  // Calculate charge spreading without magnetic field

  double sigmaq = sensor_.getBulk().K_BOLTZMANN * sensor_.getBulk().getTemperature() *
                  thickness*thickness / depletion_voltage;

  // If the bulk is n-type and the carrier are holes, then evaluate to true
  // if the bulk is p-type and the carrier are electrons, then evaluate to true
  // false otherwise
  
  if (sensor_.getBulk().isNtype() == (carrier.getCharge() == 1 ))
    sigmasq *= std::log(sumV / (sumV - common_factor));
  else
    sigmasq *= std::log((deltaV + common_factor) / deltaV);

  double sigma = std::sqrt(sigmasq);

  if (debug_)
    std::cout<<__PRETTY_FUNCTION__<<" sigma = " << sigma<<std::endl;

  // Corrections for magnetic field -- this is an approximation, may have to be done better for high fields

  // Special case if field is parallel to drift direction

  Acts::Vector3 drift_direction = drift_direction[carrier];
  Acts::Vector3 bias_surface_normal = sensor_.getBiasSurface(carrier).getNormal();

  Acts::Vector3 major_axis, minor_axis;
  double major_axis_length, minor_axis_length;

  double angular_tolerance = 1.e-9;
  
  if ( (drift_direction.cross(bias_surface_normal)).norm() < angular_tolerance) {

    
    major_axis = bias_surface_normal.cross(sensor_.getBiasSurface(carrier).getEdges().get(0).getDirection());
    minor_axis = bias_surface_normal.cross(major_axis);

    major_axis_length = sigma;
    minor_axis_length = major_axis_length;
  }
  else {

    Acts::Vector3 tmp = drift_direction_[carrier].cross(bias_surface_normal);
    major_axis = bias_surface_normal.cross(tmp);
    major_axis = major_axis / major_axis.norm();

    minor_axis = bias_surface_normal.cross(major_axis);

    // Project-to-plane would definitely be convenient here!!!

    double cos_theta_lorentz = drift_direction_[carrier].dot(bias_surface_normal);

    //drift time correction
    minor_axis_length  = sigma * (1. / cos_theta_lorentz);

    // + drift time correction
    major_axis_length  = minor_axis_length * (1. / cos_theta_lorentz);
    
  }
  
  major_axis = major_axis_length * major_axis;
  minor_axis = minor_axis_length * minor_axis;
  
  // FIXME: this has a Z component!!! (is that OK??  I think the whole thing transforms into the electrode coordinates before integrating charge.)

  Acts::Vector3 drift_destination = driftDestination(origin,carrier);
  

  GaussianDistribution2D distribution(segment_charge, drift_destination,major_axis,minor_axis);

  return distribution;
  
}

*/
