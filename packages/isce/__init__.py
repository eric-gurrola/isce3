# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# eric m. gurrola <eric.m.gurrola@jpl.nasa.gov>
# (c) 2017-2018 all rights reserved
#

# N.B. there is no python version check here; the assumption is that the isce requirements are
# in sync with the pyre requirements, so the python version check that happens on importing
# pyre is sufficient; the only problem is that the error message complains about pyre, not
# isce; is this worth fixing?

# pull the framework parts
from pyre import (
    # protocols, components and traits
    schemata, protocol, component, foundry, properties, constraints, application,
    # decorators
    export, provides,
    # the runtime manager
    executive,
    # miscellaneous packages
    geometry, patterns, primitives, timers, tracking, units,
    # support for asynchrony
    nexus,
    # plexus support
    action, command, panel, plexus, application
    )
# grab the journal
import journal

# fire up
package = executive.registerPackage(name='isce', file=__file__)
# save the geography
home, prefix, defaults = package.layout()

# export my parts
from . import (
    exceptions,     # the foundations of the ISCE exception hierarchy
    meta,           # package meta-data
    extensions,     # my extension module

    # support
    sensors,        # instruments
    topography,     # digital elevation models

    # user interfaces
    shells, actions,
    )

# my protocols
dem = topography.dem
sensor = sensors.sensor

# administrative
def copyright():
    """
    Return the isce copyright note
    """
    return print(meta.header)


def license():
    """
    Print the isce license
    """
    # print it
    return print(meta.license)


def version():
    """
    Return the isce version
    """
    return meta.version


def credits():
    """
    Print the acknowledgments
    """
    # print it
    return print(meta.acknowledgments)

# end of file
