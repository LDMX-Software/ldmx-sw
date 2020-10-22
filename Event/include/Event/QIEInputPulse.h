/**
 * @file QIEInputPulse.h
 * @brief Class for the input pulses to QIE
 * @author Niramay Gogate, Texas Tech University
 */

#ifndef EVENT_PULSE_H
#define EVENT_PULSE_H

namespace ldmx {

  /**
   * @class QIEInputPulse
   * @brief The base class to store the most important functions
   *
   * @note Always use the classes inherited from this class for 
   * QIE input pulse.
   */
  class QIEInputPulse
  {
 public:

    /**
     * Evaluate the pulse at time T
     */
    virtual float Eval(float T);

    /**
     * Integrate the pulse from T1 to T2
     */
    virtual float Integrate(float T1,float T2);

    /**
     * Differentiate pulse at time T
     */
    virtual float Derivative(float T);

    /**
     *  maximum of the pulse
     */
    virtual float Max();
  };


  /**
   * @class Bimoid
   * @brief Pulse made out of difference of two sigmoids
   *
   * @note The pulse maximum is found numerically
   */
  class Bimoid final : public QIEInputPulse
  {
 public:
    /**
     * Class constructors.
     */
    Bimoid(float start,float rise, float fall, float qq);
    Bimoid(float start,float qq);

    /**
     * Evaluate the pulse at time T
     */
    float Eval(float T) final;

    /**
     * Integrate the pulse from T1 to T2
     */
    float Integrate(float T1,float T2) final;

    /**
     *  maximum of the pulse
     */
    float Max() final;

    /**
     * Differentiate pulse at time T
     */
    float Derivative(float T) final;
 private:
    // starting time of the pulse
    float t0;
    // rise time
    float rt;
    // fall time
    float ft;
    // Normalization constant
    float NC;
    // Net charge (integral I.dt)
    float Q0;
  };

  /**
   * @class Expo
   * @brief piece-wise exponential pulse, modelled
   * as an output of a capacitor
   *
   * @note This is the preferred inpute pulse
   * shape
   */
  class Expo final : public QIEInputPulse
  {
 public:

    /**
     * The default constructor
     */
    Expo();

    /**
     * Main constructor
     * @param k_ = 1/(RC time const)
     * @param tmax_ = relative time of the pulse maximum (in ns)
     * @param tstart_ = start time of the pulse (in ns)
     * @param Q_ = Total charge carried by the pulse (in fC)
     */
    Expo(float k_,float tmax_,float tstart_,float Q_); // main constructor

    /**
     * Get Rise time of the pulse
     */
    float GetRise(){return(rt);}

    /**
     * Get Fall time of the pulse
     */
    float GetFall(){return(ft);}

    /**
     * Set Rise and Fall time of the pulse
     * @param rr Rise time
     * @param ff Fall time
     */
    void SetRiseFall(float rr,float ff);


    /**
     * Evaluate the pulse at time T
     */
    float Eval(float T) final;

    /**
     * Integrate the pulse from T1 to T2
     */
    float Integrate(float T1,float T2) final;

    /**
     *  maximum of the pulse
     */
    float Max() final;

    /**
     * Differentiate pulse at time T
     */
    float Derivative(float T) final;
 private:
    // starting time of the pulse
    float t0;
    // 1/RC time constant (for the capacitor)
    float k;
    // time when pulse attains maximum
    float tmax;
    // normalization constant (for internal use)
    float NC;
    // Rise Time
    float rt=-1;
    // Fall Time
    float ft=-1;

    /**
     * Indefinite integral at time T
     */
    float I_Int(float T);		// Indefinite integral
  };

}
#endif
