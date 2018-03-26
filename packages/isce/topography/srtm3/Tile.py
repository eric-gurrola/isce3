# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#


# externals
import enum, io, urllib.error, urllib.request, zipfile


# declaration
class Tile:
    """
    Encapsulation of an SRTMv3 tile
    """

    # error
    from .exceptions import AuthenticationError
    # types
    from .Grid import Grid as grid
    from .Availability import Availability as availability

    # public data
    point = None      # a (lat, lon) pair that locates the SW corner of the tile
    resolution = None # the number of arcseconds per pixel


    @property
    def name(self):
        """
        The canonical name of the tile, generated from its reference pixel
        """
        # first check the cache
        name = self._name
        # if it has a valid value
        if name:
            # send it off
            return name
        # otherwise, unpack the reference pixel
        lat, lon = self.point
        # build the canonical name
        name = '{}{:02}{}{:03}'.format(
            "N" if lat >=0 else "S", abs(lat),
            "E" if lon >= 0 else "W", abs(lon))
        # cache it
        self._name = name
        # and return it
        return name


    @property
    def filename(self):
        """
        The canonical name for the file that contains the tile elevation data
        """
        # easy...
        return self._filenameTemplate.format(tile=self)


    @property
    def archive(self):
        """
        The tile folder uri at the USGS archive
        """
        # easy...
        return self._usgsTemplate.format(tile=self)


    # interface
    def download(self):
        """
        Access the USGS archive and fetch the tile zip file contents
        """
        # form the uri
        uri = self.archive + self.filename
        # carefully
        try:
            # open the url
            response = urllib.request.urlopen(uri)
        # if something went wrong
        except urllib.error.HTTPError as error:
            # get the error code
            code = error.getcode()
            # 401 means that the user has not obtained authentication credentials with the archive
            if code == 401:
                # make a report
                msg = "please provide your Earthdata credentials"
                # and complain
                raise self.AuthenticationError(reason=msg)
            # 404 means the tile is not available in the dataset
            if code == 404:
                # update my status
                self.status = self.availability.unavailable
                # and return an empty byte stream
                return b''
            # let other errors flow through; someone else's problem
            raise

        # if the uri opened successfully, grab the bytes
        contents = response.read()
        # if all went well, update my status
        self.status = self.availability.available
        # and return the compressed contents
        return contents


    def read(self, cache):
        """
        Pull my compressed byte stream from the local {cache}
        """
        # my uri in the local {cache} is my {filename}
        uri = self.filename
        # pull the bytes; let errors flow through
        contents = cache[uri].open(mode='rb').read()
        # return the compressed contents
        return contents


    def write(self, cache, contents):
        """
        Save a local copy of my compressed byte stream in {contents} in the folder {cache}
        """
        # my uri in the local {cache} is my {filename}
        uri = self.filename
        # place the bytes in the cache
        cache.write(name=uri, contents=contents, mode='wb')
        # if all went well, adjust my status
        self.status = self.availability.cached
        # all done
        return self


    def decompress(self, contents):
        """
        Decompress the {contents} of the byte stream to produce the raw elevation data
        """
        # convert the contents into a zipfile
        z = zipfile.ZipFile(io.BytesIO(contents))
        # hunt down the one and only member of the archive
        hgt = z.open(z.filelist[0])
        # get the data and return it
        return hgt.read()


    def dem(self, hgt):
        """
        Convert the raw elevation byte stream in {hgt} into a properly shaped grid
        """
        # make a grid and return it
        return self.grid(hgt=hgt, resolution=self.resolution)


    # meta-methods
    def __init__(self, point, resolution=1, status=None, **kwds):
        # chain up
        super().__init__(**kwds)
        # record the reference pixel
        self.point = point
        # and the resolution
        self.resolution = resolution # arcseconds per pixel
        # deduce the status of the tile; see the {availability} enum for possible values
        self.status = self.availability.unknown if status is None else status
        # all done
        return


    # private data
    # cache for the canonical name
    _name = None
    # the tile URI template: tile name and resolution
    _filenameTemplate = "{tile.name}.SRTMGL{tile.resolution}.hgt.zip"
    # the template for the URI of the datastore at the USGS
    _usgsTemplate = "http://e4ftl01.cr.usgs.gov/MEASURES/SRTMGL{tile.resolution}.003/2000.02.11/"


# end of file
