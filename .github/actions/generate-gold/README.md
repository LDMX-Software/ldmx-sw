# generate-gold Action

An action to run the specified config with the most recently generated production image.

**We do not checkout the `trunk` branch so this will break if specific changes are made to python interaction of fire.** 

After this action is run, further calls using the `ldmx` action can be done to use this compiled version of ldmx-sw to do things.

## Inputs

- `config`: **required** 
  - The config to run and generate the "golden" histograms.
  - Specify this config with respect to the `.github/workflows` directory.
  - For example: `configs/inclusive.py`

## In-Workflow Example

This is meant to be pared with the caching action, so we don't have to "generate gold" as frequently.

```yaml
    - name: Cache the Gold
      id: cache-gold
      uses: actions/cache@v2
      with:
        path: .github/workflows/gold/inclusive.root
        key: inclusive-gold

    - name: Generate Gold
      if: steps.cache-gold.outputs.cache-hit != 'true'
      uses: ./.github/actions/generate-gold
      with:
        config: configs/inclusive.py
```
