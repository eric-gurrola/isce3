# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis
# orthologue
# (c) 2003-2018 all rights reserved
#

"""
Definitions for all the exceptions raised by this package
"""


# pull the base class
from isce.exceptions import ISCEError


# my local error types
class AuthenticationError(ISCEError):
    """
    Exception raised when there is something wrong with the user's credentials
    """

    # public data
    description = "{0.reason}"
    reason = "unknown authentication error"


    # meta-methods
    def __init__(self, reason=None, **kwds):
        # chain up
        super().__init__(**kwds)
        # if the caller supplied a {reason} for the error
        if reason is not None:
            # save it
            self.reason = reason
        # all done
        return


# end of file
