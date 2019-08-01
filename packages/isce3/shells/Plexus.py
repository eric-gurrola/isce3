#-*- coding: utf-8 -*-


# access the pyre framework
import pyre
# and my package
import isce3


# build my CLI action dispatcher
class Plexus(pyre.plexus, family="isce3.components.plexus"):
    """
    The main CLI action dispatcher
    """


    # install my action protocol
    from .Action import Action as pyre_action


    # pyre framework hooks
    def pyre_banner(self):
        """
        Generate the help banner
        """
        # show the license header
        return isce3.meta.license


# end of file
