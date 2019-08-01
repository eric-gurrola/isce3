#-*- coding: utf-8 -*-


# access the framework
import pyre
# my protocol
from .Action import Action as action


# pull {panel} and rebrand it
class Command(pyre.panel(), implements=action):
    """
    Base class for {isce3} commands
    """


# end of file
