# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis
# orthologue
# (c) 2003-2018 all rights reserved
#


# externals
import urllib.error
# base class
from pyre.nexus.Crew import Crew


# the task that fetches tiles from the SRTM archive
class Retriever(Crew):
    """
    Download tiles from the SRTM archive
    """

    # types
    from .Availability import Availability as tilecodes


    # interface
    def engage(self, task, **kwds):
        """
        Download a tile
        """
        # grab a journal channel
        channel = self.info
        # my tasks are tiles
        tile = task
        # sign in
        channel.log("{.pid}: fetching tile {.name}".format(self, tile))
        # alias the task availability enum
        codes = self.tilecodes
        # grab the availability map
        map = self.map
        # look up my tile
        map.getTileAvailability(tile=tile)

        # if the tile is known to not exist
        if tile.status is codes.unavailable:
            # tell me
            channel.log("{.pid}: tile {.name} is unavailable".format(self, tile))
            # not much else to do
            return 0
        # if the tile is already cached locally
        if tile.status is codes.cached:
            # tell me
            channel.log("{.pid}: tile {.name} is already cached".format(self, tile))
            # all done
            return 0

        # fetch the tile
        data = tile.download()

        # if the status is {unknown}, something went wrong during our interaction with the archive
        if tile.status is codes.unknown:
            # tell me
            channel.line("{.pid}: could not fetch tile {.name}".format(self, tile))
            channel.log("  perhaps you are not authorized to access the data archive")
            # and bail
            return 0

        # update the map with what we know at this point
        map.update(tile=tile)

        # if the status is now {unavailable}
        if tile.status is codes.unavailable:
            # tell me
            channel.log("{.pid}: tile {.name} is unavailable".format(self, tile))
            # we are done
            return 0

        # otherwise, we have tile data; cache it
        tile.write(cache=self.cache, contents=data)
        # and update the map once more
        map.update(tile=tile)

        # tell me
        channel.log("{.pid}: tile {.name} is now cached".format(self, tile))

        # indicate success
        return 0


    # meta-methods
    def __init__(self, cache, map, **kwds):
        # chain up
        super().__init__(**kwds)
        # save the location of the local archive
        self.cache = cache
        # and the availability map
        self.map = map
        # all done
        return


# end of file
