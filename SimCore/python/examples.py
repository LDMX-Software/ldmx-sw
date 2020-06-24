"""Examples of using different primary generators"""

from LDMX.SimCore import generators
from LDMX.SimCore import simulator

def inclusive_single_e() :
    """Get a basic un-biased, inclusive single electron simulation

    This function is mainly helpful for testing that your install
    works with this primary generator. You can look at the 
    source code for this function for ideas on how to implement your own
    verison.

    Returns
    -------
    ldmxcfg.Producer
        that will simulate un-biased, inclusive single electrons
        fired from upstream of the tagger into the v12 ldmx geometry

    Examples
    --------
        bkgd_sim = examples.inclusive_single_e()
    """

    sim = simulator.simulator( "inclusive_single_e" )
    sim.setDetector( "ldmx-det-v12" )
    sim.runNumber = 1
    sim.description = "One 4GeV electron shot from far upstream."
    sim.generators.append(generators.single_4gev_e_upstream_tagger())
    return sim

def lheExample( lheFile ) :
    """An example simulator of how to use the LHE generator

    This function is mainly helpful for testing that your install
    works with this primary generator. You can look at the 
    source code for this function for ideas on how to implement your own
    verison.

    Parameters
    ----------
    lheFile : str
        The LHE file to use a primary vertices

    Returns
    -------
    ldmxcfg.Producer
        that will simulate the v12 detector using the 
        input LHE file with high verbosity.

    Example
    -------
        lhe_sim = examples.lheExample( thePathToMyLHEFile )
    """

    sim = simulator.simulator( "lheSimulation" )
    sim.setDetector( "ldmx-det-v12" )
    sim.runNumber = 1
    sim.description = "Example of how to use LHE generator"
    sim.generators.append(generators.lhe( "LHEExample" , lheFile ))
    return sim

def gpsExample( ) :
    """An example simulator of how to use the General Particle Source.

    This function is mainly helpful for testing that your install
    works with this primary generator. You can look at the 
    source code for this function for ideas on how to implement your own
    verison.

    Returns
    -------
    ldmxcfg.Producer
        that will simulate the v12 detector with
        high verbosity and an electron smeared across the target
        with a cos angular dependence and a linear energy dependence
        between 3 and 4 GeV.

    Example
    -------
        gps_sim = examples.gpsExample()
    """

    sim = simulator.simulator( "gpsExample" )
    sim.setDetector( "ldmx-det-v12" )
    sim.runNumber = 1
    sim.description = "Example of how to use GPS generator"
    gpsGen = generators.gps( "GPSExample" , [
        "/gps/particle e-",
        "/gps/pos/type Plane",
        "/gps/pos/shape Square",
        "/gps/pos/centre 0 0 0 mm",
        "/gps/pos/halfx 40 mm",
        "/gps/pos/halfy 80 mm",
        "/gps/ang/type cos",
        "/gps/ene/type Lin",
        "/gps/ene/min 3 GeV",
        "/gps/ene/max 4 GeV",
        "/gps/ene/gradient 1",
        "/gps/ene/intercept 1"
        ])
    sim.generators.append(gpsGen)
    return sim

def multiExample( ) :
    """An example simulator of how to use the Multi-Primary Particle Gun

    This function is mainly helpful for testing that your install
    works with this primary generator. You can look at the 
    source code for this function for ideas on how to implement your own

    Returns
    -------
    ldmxcfg.Producer
        Simulator in the v12 geometry with high verbosity and 4GeV electrons
        fired from within the target where the number of electrons is
        Poisson-distributed around an average of 2

    Example
    -------
        mpg_sim = examples.multiExample()
    """

    sim = ldmxcfg.Producer( "mpgExample" , "ldmx::Simulator" )
    sim.setDetector( "ldmx-det-v12" )
    sim.runNumber = 1
    sim.description = "Example of how to use MPG generator"
    mpgGen = generators.multi( "MPGExample" )
    mpgGen.vertex = [ 0., 0., 0. ] #mm
    mpgGen.nParticles = 2
    mpgGen.pdgID = 11
    mpgGen.enablePoisson = True
    mpgGen.momentum = [ 0., 0., 4000. ]#MeV
    sim.generators.append(mpgGen)
    return sim

