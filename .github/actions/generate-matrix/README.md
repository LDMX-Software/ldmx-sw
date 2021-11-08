# generate-matrix Action

Generate a JSON job matrix for all the samples in `.github/validation_samples` 
and the input number of runs.

## Inputs

`num_jobs_per_sample` : optional
  - Number of jobs per sample to generate
  - Default: `1`
  - Postive integer, run numbers sequentially count up from 1 to this number.
