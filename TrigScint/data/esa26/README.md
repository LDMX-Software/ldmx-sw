# ESA26 trigger-scintillator rechit calibration

`calibration_pad{1,2,3}.txt` are read by `TrigScintRecHitProducer`
(`use_calib_file=True`), one row per bar: `bar gain[e/PE] pedestal[fC/sample]`.

Source: Run 319, TS-only readout, per-channel Gaussian fits of 10-sample charge
sums (peak 0 = pedestal, peak 1 = 1 PE). For each channel

- gain = (mu1 - mu0) [fC/PE] x 6250 e/fC
- pedestal = mu0 / 10 [fC/sample]

(lane, channel) is mapped to (pad, bar) with `../channelMap_4modules_14lanes.txt`
and `../moduleMap_4modules_14lanes.txt` (pad = module + 1, lanes 4(p-1) to 4p-1).