# check Action

Check if we passed the comparison or not.

This is just here to isolate it. 
Essentially, we are provided a short-name for a sample and we check if there are any plots in its `fail` directory. 
If there are, we exit with a non-zero exit code.

**This depends entirely on knowing the path to the comparison plots.**

## Inputs

- `sample_id` : **required**
  - String short-name for the sample to check

