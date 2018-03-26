# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# the package
import isce
# the extension module with the low level functions
import isce.extensions.isce as libisce


# the tile availability map
class AvailabilityMap:
    """
    The tile availability map
    """

    # types
    from .Availability import Availability as availability


    # interface
    def sync(self, cache, mosaic, channel=None, indent=1):
        """
        Sync my contents with the contents of the local {cache} for the tiles in {mosaic}
        """
        # make a channel
        channel = self._channel if channel is None else channel
        # setup the margin
        margin = '  '*indent
        # sign in
        channel.line(f'{margin}updating the availability map')

        # now visit all the tiles in the globe
        for tile in mosaic:
            # check whether the tile exists in the local store
            isCached = tile.name in cache
            # fetch the map status of this tile
            status = self.getTileAvailability(tile=tile)

            # do the update
            # if the file is in the local store but the map doesn't know it
            if isCached and status is not self.availability.cached:
                # adjust the tile status
                tile.status = self.availability.cached
                # mark it in the map
                self.update(tile=tile)
                # and show me
                channel.line(f'{margin}  marked tile {tile.name!r} as locally available')

            # if the file was marked as cached in the map but it's not in the store contents
            if status is self.availability.cached and not isCached:
                # downgrade it to available but not cached
                tile.status = self.availability.available
                # update the map
                self.update(tile=tile)
                # and show me
                channel.line(f'{margin}  tile {tile.name!r} has disappeared')

        # all done
        return


    def summary(self, channel=None, dent=0):
        """
        Produce a tile availability summary
        """
        # make a channel
        channel = self._channel if channel is None else channel
        # setup the margin
        margin = '  '*dent
        # sign in
        channel.line(f'{margin}tile availability summary:')

        # get my map
        map = self.map
        # and compute the range of status values
        range = len(self.availability)
        # build the summary
        frequencies = libisce.srtmAvailabilityMapSummary(map, range)

        # figure out the widest status label
        width = max(len(status.name) for status in self.availability)

        # display the table: go through the status codes
        for status in self.availability:
            # label
            label = status.name
            # compute the count
            count = frequencies[status.value]
            # why not do it right...
            plural = '' if count == 1 else 's'
            # show me
            channel.line(f'{margin}  {label:>{width}}: {count} tile{plural}')

        # all done
        return frequencies


    # individual tile status access
    def getTileAvailability(self, tile):
        """
        Check the availability of the given tile
        """
        # get its location
        index = tile.point
        # get the status from my map
        value = libisce.srtmAvailabilityMapGet(self.map, index)
        # convert it into an availability code
        status = tuple(self.availability)[value]
        # mark the tile
        tile.status = status
        # and return the code
        return status


    def update(self, tile):
        """
        Adjust the status of the given tile
        """
        # get its location
        index = tile.point
        # get the status value
        value = tile.status.value
        # set the status in my map and return it
        return libisce.srtmAvailabilityMapSet(self.map, index, value)


    # meta-methods
    def __init__(self, uri, **kwds):
        # chain up
        super().__init__(**kwds)
        # save my path
        self.uri = uri
        # convert the path into a string and make the capsule
        self.map = libisce.srtmAvailabilityMap(str(uri))

        # make a default channel
        self._channel = isce.journal.info("isce.topography.srtm")

        # all done
        return


# end of file
