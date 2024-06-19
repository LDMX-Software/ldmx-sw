# LDMX Visualisation

## Installation

For running the visualiser, you need Node.js and Yarn.

Once you have Node.js, run

```npm install --global yarn```

You might need to set your Yarn version to Yarn 2 and beyond with

```yarn set version berry```

Then run

```yarn install```

and

```yarn start```

The visualiser can now be accessed at localhost:4200.

## Known issues

Different clusters are supposed to have different colors, and cluster centroids are supposed to be the same color as the hits included in that cluster. However, Phoenix has a bug where it cannot load colors from a JSON file for a Box type object. Waiting for this to be fixed, meanwhile the different cluster collections can still be manually set to different colors using the Phoenix menu at the top right.