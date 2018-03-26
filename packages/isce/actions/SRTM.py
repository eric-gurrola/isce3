# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis
# orthologue
# (c) 1998-2015 all rights reserved
#


# externals
import isce


# declaration
class SRTM(isce.panel(), family='isce.actions.srtm'):
    """
    Manage the local cache of SRTM elevation tiles

    The SRTM tiles necessary to assemble digital elevation models are kept in a local
    cache. Subsequent processing that requires these tiles can be serviced from your disk,
    saving a trip to the USGS server.

    You can get information about your local store by issuing the 'sync' command, which
    will tell you where the store is and generate a report of its current contents.

    If you know your region of interest, you can ask for a work plan, to get a feeling for
    which tiles cover it and their status. You can also download them to avoid network
    traffic during later processing. Regions of interest are formed by computing the
    bounding box of a list of (lat, lon) pairs.

    You must have an account with Earthdata in order to retrieve tiles that are not in the
    local store. Visit

        https://urs.earthdata.nasa.gov/users/new

    to get one. For your convenience, you can store your credentials by issuing the 'auth'
    command so that you don't have to supply them on the command line every time you need to
    access the archive.
    """


    # user configurable state
    srtm = isce.topography.dem(default=isce.topography.srtm())
    srtm.doc = 'the digital elevation model assembler'

    username = isce.properties.str(default=None)
    username.doc = 'your Earthdata user name'

    password = isce.properties.str(default=None)
    password.doc = 'your Earthdata password'

    region = isce.properties.array()
    region.doc = 'an array of (lat, lon) pairs of interest'

    resolution = isce.properties.int(default=1)
    resolution.doc = 'the resolution of the data set in pixels per arc-second; either 1 or 3'
    resolution.validators = isce.constraints.isMember(1,3)

    force = isce.properties.bool(default=False)
    force.doc = 'perform the requested action unconditionally'


    # commands
    @isce.export(tip="store Earthdata credentials")
    def auth(self, plexus, **kwds):
        """
        Collect and store Earthdata login credentials for use during subsequent tile requests
        """
        # get a channel
        channel = plexus.info
        # show me
        channel.line('storing authentication credentials')
        # get my data store manager
        srtm = self.srtm
        # and ask it to process the user's credentials
        srtm.authenticate(credentials=(self.username, self.password), channel=channel)
        # flush
        channel.log()
        # and return the status code
        return 0


    # commands
    @isce.export(tip="sync the availability map with the contents of the local cache ")
    def sync(self, plexus, **kwds):
        """
        Ensure that the availability map reflects the contents of the local store accurately
        """
        # pick a channel
        channel = plexus.info
        # sign in
        channel.line('syncing the local availability map')
        # get my data store manager
        srtm = self.srtm
        # and ask it to synchronize the availability map with the contents of the archive
        status = srtm.sync(channel=channel, dent=1)
        # flush the channel
        channel.log()
        # and return the status code
        return status


    @isce.export(tip="make a workplan for assembling the elevation model for the given region")
    def plan(self, plexus, **kwds):
        """
        Describe the workplan to build an elevation model for the region of interest
        """
        # pick a channel
        channel = plexus.info
        # sign in
        channel.line('making a work plan')
        # get my data store manager
        srtm = self.srtm
        # and ask it to make a plan
        status = srtm.plan(channel=channel, dent=1)
        # flush the channel
        channel.log()
        # and return the status code
        return status


    @isce.export(tip="download the tiles that cover a cloud of points")
    def download(self, plexus, **kwds):
        """
        Download tiles from the SRTM archive

        The {region} of interest is specified as a cloud of (lat, lon) pairs. This command
        computes the bounding box the points, and downloads all the tiles necessary to cover it
        """
        # pick a channel
        channel = plexus.info
        # sign in
        channel.line('retrieving tiles')
        # get my data store manager
        srtm = self.srtm
        # and ask it to download the relevant tiles
        status = srtm.download(channel=channel, credentials=(self.username, self.password))
        # flush the channel
        channel.log()
        # and return the status code
        return status


    @isce.export(tip="generate the elevation model for the region of interest")
    def generate(self, plexus, **kwds):
        """
        Generate the elevation model for the region of interest
        """
        # get the archive accessor
        srtm = self.srtm
        # configure it
        srtm.region = self.region
        srtm.force = self.force
        # ask it to generate the elevation model
        status = srtm.generate()
        # all done
        return status


    # meta-methods
    def __init__(self, plexus, **kwds):
        # chain up
        super().__init__(plexus=plexus, **kwds)
        # grab the plexus private file space
        pfs = plexus.pfs

        # get my data store manager
        srtm = self.srtm

        # establish the location of the data store
        # the default location of the data set is derived from the manager family name
        family = srtm.pyre_familyFragments()
        # if the store manager has the proper pedigree
        if family:
            # derive the location of the data store from its name by dropping the package name
            uri = isce.primitives.path(family[1:])
            # access it; careful not to look too deeply just in case there is a lot of data
            # cached here
            cache = pfs['etc'].mkdir(uri).discover(levels=1)
            # attach it
            srtm.cache = cache

        # pass on the region
        srtm.region = self.region
        # and the desired resolution
        srtm.resolution = self.resolution

        # all done
        return


# end of file
