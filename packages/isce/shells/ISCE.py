# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#


# support
import isce


# the ISCE plexus
class ISCE(isce.plexus, family='isce.shells.isce'):
    """
    The main action dispatcher for the isce application
    """

    # types
    from .Action import Action as pyre_action


    # pyre framework hooks
    # support for the help system
    def pyre_banner(self):
        """
        Place the application banner in the {info} channel
        """
        # show the license header
        return isce.meta.license


    # interactive session management
    def pyre_interactiveSessionContext(self, context):
        """
        Go interactive
        """
        # protect against bad contexts
        if context is None:
            # by initializing as an empty dict
            context = {}

        # set up some context
        context['isce'] = isce  # my package

        # and chain up
        return super().pyre_interactiveSessionContext(context=context)


# end of file
