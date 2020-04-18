/**
 * @file EcalHitIO.h
 * @brief Class providing hit readout for simulated LDMX ECal detector
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Owen Colegro, UCSB
 */

#ifndef SIMAPPLICATION_ECALHITIO_H_
#define SIMAPPLICATION_ECALHITIO_H_

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "Event/SimCalorimeterHit.h"
#include "SimApplication/G4CalorimeterHit.h"
#include "SimApplication/SimParticleBuilder.h"

// STL
#include <vector>
#include <utility>

namespace ldmx {

    /**
     * @class EcalHitIO
     * @brief Provides hit readout for simulated ECal detector
     *
     * @note
     * This class uses the EcalHexReadout to transform the G4CalorimeterHit hits collection
     * into a collection of SimCalorimeterHit objects assigned to hexagonal cells by their ID
     * and positions.  Energy depositions in the same cells are combined together into single
     * hits.
     *
     * @par
     * It can be run in three modes:
     * <ul>
     * <li>Compressed hit contributions combined by matching SimParticle and PDG code (default mode)
     * <li>Full hit contributions with one record per step (when compressHitContribs_ is false)
     * <li>No hit contribution information where energy is combined but vectors are not filled (when enableHitContribs_ is false)
     * </ul>
     */
    class EcalHitIO {

        public:

            /**
             * Layer-cell pair.
             */
            typedef std::pair<int, int> LayerCellPair;

        public:

            /**
             * Class constructor.
             */
            EcalHitIO() { 
        
                // These are the v12 parameters
                //  all distances in mm
                double moduleRadius = 85.0; //same as default
                int    numCellsWide = 23; //same as default
                double moduleGap = 1.0;
                double ecalFrontZ = 220;
                std::vector<double> ecalSensLayersZ = {
                     7.850,
                    13.300,
                    26.400,
                    33.500,
                    47.950,
                    56.550,
                    72.250,
                    81.350,
                    97.050,
                    106.150,
                    121.850,
                    130.950,
                    146.650,
                    155.750,
                    171.450,
                    180.550,
                    196.250,
                    205.350,
                    221.050,
                    230.150,
                    245.850,
                    254.950,
                    270.650,
                    279.750,
                    298.950,
                    311.550,
                    330.750,
                    343.350,
                    362.550,
                    375.150,
                    394.350,
                    406.950,
                    426.150,
                    438.750
                };
        
                hexReadout_ = std::make_unique<EcalHexReadout>(
                        moduleRadius,
                        moduleGap,
                        numCellsWide,
                        ecalSensLayersZ,
                        ecalFrontZ
                        );
            }

            /**
             * Class destructor.
             */
            ~EcalHitIO() { }

            /**
             * Write out a Geant4 hits collection to the provided ROOT array.
             * @param hc The input hits collection.
             * @param outputColl The output collection in ROOT.
             */
            void writeHitsCollection(G4CalorimeterHitsCollection* hc, std::vector<SimCalorimeterHit> &outputColl);

            /**
             * Set whether hit contributions should be enabled for the output hits.
             * @param enableHitContribs True to enable hit contributions.
             */
            void setEnableHitContribs(bool enableHitContribs) {
                enableHitContribs_ = enableHitContribs;
            }

            /**
             * Set whether hit contributions should be compressed by particle and PDG code.
             * @param compressHitContribs True to compress hit contribution information.
             */
            void setCompressHitContribs(bool compressHitContribs) {
                compressHitContribs_ = compressHitContribs;
            }

        private:

            /**
             * Hex cell readout.
             *
             * Inputs v12 geometry parameters
             */
            std::unique_ptr<EcalHexReadout> hexReadout_;

            /**
             * Enable hit contribution output.
             */
            bool enableHitContribs_ {true};

            /**
             * Enable compression of hit contributions by SimParticle and PDG code.
             */
            bool compressHitContribs_ {true};
    };

} // namespace sim

#endif
