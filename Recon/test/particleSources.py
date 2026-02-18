

def get_cmds(part='e-', energy_range=(0.1,3.), theta_range_deg=(0.,20.), zpos=0.):
    from math import pi
    rad_near_beam = pi - pi/180. * theta_range_deg[0]
    rad_far_beam  = pi - pi/180. * theta_range_deg[1]

    return [
    "/gps/particle "+part,
    "/gps/pos/type Plane",
    "/gps/direction 0 0 1",
    # Mono energy
    # "/gps/ene/mono 4 GeV",
    # Linear energy
    "/gps/ene/type Lin",
    f"/gps/ene/min {energy_range[0]} GeV",
    f"/gps/ene/max {energy_range[1]} GeV",
    "/gps/ene/gradient 0",
    "/gps/ene/intercept 1",
    # circle
    # "/gps/pos/shape Circle",
    # "/gps/pos/centre 0 0 240 mm",
    # "/gps/pos/radius 50 mm", #50 or 150
    # Square
    "/gps/pos/shape Square",
    f"/gps/pos/centre 0 0 {zpos} mm",
    "/gps/pos/halfx 1 mm",
    "/gps/pos/halfy 1 mm",
    # angles
    # "/gps/ang/type cos",
    "/gps/ang/type iso",
    f"/gps/ang/mintheta {rad_far_beam} rad",
    f"/gps/ang/maxtheta {rad_near_beam} rad",
    # number of particles
    #"/gps/number "+str(n_part), # shoots at same location
]

cocktail_commands=[]
cocktail_commands += get_cmds('e-')
cocktail_commands += ['/gps/source/add 1', *get_cmds('e+')]
cocktail_commands += ['/gps/source/add 1', *get_cmds('gamma')]
cocktail_commands += ['/gps/source/add 1', *get_cmds('pi-')]
cocktail_commands += ['/gps/source/add 1', *get_cmds('pi+')]
cocktail_commands += ['/gps/source/add 1', *get_cmds('mu-')]
cocktail_commands += ['/gps/source/add 1', *get_cmds('mu+')]
cocktail_commands += ['/gps/source/add 1', *get_cmds('proton')]
cocktail_commands += ['/gps/source/add 1', *get_cmds('neutron')]
cocktail_commands += ['/gps/source/multiplevertex False']  # choose 1 randomly
