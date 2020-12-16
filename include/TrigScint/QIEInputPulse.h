/**
 * @file QIEInputPulse.h
 * @brief Class for the input pulses to QIE
 * @author Niramay Gogate, Texas Tech University
 */

#include<vector>

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
     * Evaluate the pulse train at time T
     */
    float Eval(float T);

    /**
     * Evaluate the pulse train at time T
     */
    virtual float EvalSingle(float T, int id) = 0;

    /**
     * Integrate the pulse from T1 to T2
     */
    virtual float Integrate(float T1,float T2) = 0;

    /**
     * Differentiate pulse at time T
     */
    virtual float Derivative(float T, int id) = 0;

    /**
     *  maximum of the pulse
     */
    virtual float Max(int id) = 0;

    /**
     * To add a pulse to the collection
     * @param toff time at which the pulse starts
     * @param ampl pulse amplitude (total area under the curve)
     */
    void AddPulse(float toff, float ampl);

    /// collection of pulse time offsets
    std::vector<float> toff_;

    /// collection of pulse amplitudes
    std::vector<float> ampl_;

    /// no. of pulses in the collection
    int npulses{0};
  };


  /**
   * @class Bimoid
   * @brief Pulse made out of difference of two sigmoids
   *
   * @note The pulse maximum is found numerically
   */
  class Bimoid : public QIEInputPulse
  {
 public:
    /**
     * Class constructors.
     */
    Bimoid(float start,float qq);

    /**
     * Default class destructor
     */
    ~Bimoid(){};
    
    /**
     * Evaluate the pulse at time T
     */
    float EvalSingle(float T, int id) final;

    /**
     * Integrate the pulse from T1 to T2
     */
    float Integrate(float T1,float T2) final;

    /**
     *  maximum of the pulse
     */
    float Max(int id) final;

    /**
     * Differentiate pulse at time T
     */
    float Derivative(float T, int id) final;
 private:
    /// rise time
    float rt;
    /// fall time
    float ft;
  };

  /**
   * @class Expo
   * @brief piece-wise exponential pulse, modelled
   * as an output of a capacitor
   *
   * @note This is the preferred inpute pulse
   * shape
   */
  class Expo : public QIEInputPulse
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
     */
    Expo(float k_,float tmax_); /// main constructor

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
    float EvalSingle(float T, int id) final;

    /**
     * Integrate the pulse from T1 to T2
     */
    float Integrate(float T1,float T2) final;

    /**
     *  maximum of the pulse
     */
    float Max(int id) final;

    /**
     * Differentiate pulse at time T
     */
    float Derivative(float T, int id) final;
 private:
    /// 1/RC time constant (for the capacitor)
    float k;
    /// time when pulse attains maximum
    float tmax;
    /// Rise Time
    float rt=-1;
    /// Fall Time
    float ft=-1;

    /**
     * Indefinite integral at time T
     */
    float I_Int(float T, int id);
  };

}
#endif
