#ifndef TOOLS_PULSERECORD_H_
#define TOOLS_PULSERECORD_H_

// new class PulseRecord for plotting purposes

namespace ldmx {

class PulseRecord {
 public:
  // Constructor to initialize Voltage, ADC, and TOT
  PulseRecord(double volts, int adc, int tot)
      : volts_(volts), adc_(adc), tot_(tot) {}

  // Getters for the recorded data
  double getVolts() const { return volts_; }
  int getADC() const { return adc_; }
  int getTOT() const { return tot_; }

 private:
  double volts_;
  int adc_;
  int tot_;
};
}  // namespace ldmx

#endif