# validate Action

An action to validate the specified sample.

## Assumptions

Each "sample" is a subdirectory of `.github/validation_samples` with the following structure.

- `.github/validation_samples/`
  - `<sample-name>/`
    - `config.py`
    - `gold.root`

where `config.py` generates the _histogram file_ `hist.root` 
when run with ldmx-sw. `gold.root` is a copy of `hist.root` when `config.py`
was generated with a "verified" version of ldmx-sw (whatever that means).

## Inputs

- `sample`: **required** 
  - The name of the sample to validate (i.e. the subdirectory name `<sample-name>`)
  - For example: `inclusive`

