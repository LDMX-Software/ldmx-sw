from LDMX.Framework import ldmxcfg

class PositionAccuracy(ldmxcfg.Producer) :
    def __init__(self,name) :
        super().__init__(name,'trigscint::PositionAccuracy', 'TrigScint')
        self.input_collections = ["TriggerPad1Clusters", "TriggerPadTracksY", "BeamElectronTruthInfoPad1"]
        self.output_collections = ["ClusterPositionDifferencePad1", "TrackPositionDifferencePad1"]
        self.input_pass_name = ""   # Take any pass
        self.vertical_bar_start_index = 52
        self.padNumber = 1
        self.padPosition = [-21.5,0.,-876.]
        self.verbose = False

    def pad1() :
        diff = PositionAccuracy('PositionDifferencePad1')
        diff.input_collections = ["TriggerPad1Clusters", "TriggerPadTracksY", "BeamElectronTruthInfoPad1"]
        diff.output_collections = ["ClusterPositionDifferencePad1", "TrackPositionDifferencePad1"]
        diff.padNumber = 1
        diff.padPosition = [-21.5,0.,-876.]

        return diff

    def pad2() :
        diff = PositionAccuracy('PositionDifferencePad2')
        diff.input_collections = ["TriggerPad2Clusters", "TriggerPadTracksY", "BeamElectronTruthInfoPad2"]
        diff.output_collections = ["ClusterPositionDifferencePad2", "TrackPositionDifferencePad2"]
        diff.padNumber = 2
        diff.padPosition = [-19.,0.,-816.]

        return diff

    def pad3() :
        diff = PositionAccuracy('PositionDifferencePad3')
        diff.input_collections = ["TriggerPad3Clusters", "TriggerPadTracksY", "BeamElectronTruthInfoPad3"]
        diff.output_collections = ["ClusterPositionDifferencePad3", "TrackPositionDifferencePad3"]
        diff.padNumber = 3
        diff.padPosition = [0.,0.,-2.4262]

        return diff
