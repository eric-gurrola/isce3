#-*- Python -*-
#-*- coding: utf-8 -*-


# import and publish pyre symbols
from pyre import (
    # protocols, components, traits, and their infrastructure
    schemata, constraints, properties, protocol, component, foundry,
    # decorators
    export, provides,
    # the manager of the pyre runtime
    executive,
    # support for concurrency
    nexus,
    # support for multidimensional containers
    grid,
    # miscellaneous
    tracking, units
    )


# bootstrap
package = executive.registerPackage(name='isce3', file=__file__)
# save the geography
home, prefix, defaults = package.layout()


# publish the local modules
from . import (
    meta,        # package meta-data
)


# administrative
def copyright():
    """
    Print the isce3 copyright note
    """
    return print(meta.header)


def license():
    """
    Print the isce3 license
    """
    # print it
    return print(meta.license)


def version():
    """
    Return the isce3 version
    """
    return meta.version


def credits():
    """
    Print the acknowledgments
    """
    # print it
    return print(meta.acknowledgments)


# end of file
