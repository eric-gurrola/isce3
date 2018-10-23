# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# eric m. gurrola <eric.m.gurrola@jpl.nasa.gov>
# (c) 2017-2018

# access to the toppography downloading library (needs to be integrated in the current project)
# publish my parts
#try:
#    from . import isce as libisce
#except ImportError:
#    print("unable to import libisce (dem downloader)")

# access to iscecore library
try:
    from . import isceextension as libiscecore
except ImportError:
    print("unable to import libiscecore")
else:
    # Note: We can use this __init__ to protect the "py-tagged" namespace of the C++-to-Python
    #       objects, while still being able to call 'from iscecore import Orbit' e.g. instead
    #       of 'from iscecore import pyOrbit'
    Ellipsoid = libiscecore.pyEllipsoid
    Interpolator = libiscecore.pyInterpolator
    Peg = libiscecore.pyPeg
    Pegtrans = libiscecore.pyPegtrans
    Position = libiscecore.pyPosition
    LinAlg = libiscecore.pyLinAlg
    Poly1d = libiscecore.pyPoly1d
    Poly2d = libiscecore.pyPoly2d
    Orbit = libiscecore.pyOrbit


'''     CURRENTLY DEPRECATED
    DateTime = pyDateTime
'''

# end-of-file
