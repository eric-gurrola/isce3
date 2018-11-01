# -*- Python -*-
# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis <michael.aivazis@para-sim.com>
# (c) 2003-2018 all rights reserved
#

# get the package
import isce
# externals
import collections, functools, getpass, os, pickle, urllib.request


# the digital elevation model protocol
class SRTM(isce.component,
           family='isce.topography.dem.srtm', implements=isce.topography.dem):
    """
    Access the SRTM data archive to download tiles and produce a digital elevation model for a
    specified region of interest
    """

    # errors
    from .exceptions import AuthenticationError
    # types
    from .Mosaic import Mosaic as mosaic
    from .Availability import Availability as availability


    # user configurable state
    region = isce.properties.array()
    region.doc = 'a cloud of (lat,lon) pairs that specify of the region of interest'

    resolution = isce.properties.int(default=1)
    resolution.doc = 'the resolution of the data set in pixels per arc-second; either 1 or 3'
    resolution.validators = isce.constraints.isMember(1,3)

    team = isce.nexus.team()
    team.doc = 'the manager of the distributed pool of workers'

    pool = isce.properties.int(default=8)
    pool.doc = 'the maximum number of simultaneous connections to the remote data store'


    # protocol obligations
    @isce.export
    def authenticate(self, credentials=None, channel=None):
        """
        Retrieve and store the user's authentication credentials
        """
        # unpack
        username, password = credentials
        # if the user did not supply a username
        if username is None:
            # ask for it
            username = input("username: ")

        # if the user did not supply a password
        if password is None:
            # ask for it
            password = getpass.getpass("password: ")

        # get my cache
        cache = self.cache
        # attempt to
        try:
            # grab the credential database
            authdb = cache[self._authdb]
        # if it doesn't exist
        except cache.NotFoundError:
            # make it
            authdb = cache.write(name=self._authdb, contents='')
            # and make an empty credentials database
            db = {}
        # if it does
        else:
            # attempt to
            try:
                # pull in its contents
                db = pickle.load(authdb.open(mode="rb"))
            # if something goes wrong
            except EOFError:
                # no worries: make an empty db
                db = {}

        # store the credentials
        db[username] = password
        # and persist
        pickle.dump(db, authdb.open(mode="wb"))

        # all done
        return


    @isce.export
    def sync(self, channel=None, dent=0):
        """
        Ensure that the availability map reflects the contents of the local store accurately
        """
        # make a channel
        channel = isce.journal.info(self.pyre_family()) if channel is None else channel

        # get the map
        map = self.availabilityMap
        # build a mosaic over the entire globe
        globe = self.mosaic(region=[(-90,-180), (89,179)], resolution=self.resolution)
        # get the contents of the local store as a set of tile names
        contents = self.localstoreContents
        # do the sync
        map.sync(cache=contents, mosaic=globe, channel=channel)

        # everything below is a report
        if not channel:
            # so skip it if the channel is not active
            return

        # compute the margin
        margin = '  '*dent
        # show me where the local store is
        channel.line(f"{margin}local cache: '{self.cache.uri}'")
        # show me the name of the availability map
        channel.line(f"{margin}      map: '{map.uri.name}'")
        # count the number of tiles that are present
        count = len(contents)
        # why not do it right...
        plural = '' if count == 1 else 's'
        # show me
        channel.line(f'{margin}    globe: {len(globe)} tiles')
        channel.line(f'{margin}  present: {count} tile{plural}')
        # ask the map for a summary
        map.summary(channel=channel, dent=dent)

        # all done
        return


    @isce.export
    def plan(self, channel=None, dent=0):
        """
        Describe the work required to generate the specified DEM
        """
        # get my region
        region = self.region
        # build a mosaic that covers it
        mosaic = self.mosaic(region=self.region, resolution=self.resolution)
        # get the map
        availability = self.availabilityMap
        # check the status of the tiles in the mosaic
        summary = self.checkAvailability(mosaic=mosaic)

        # make a channel
        channel = isce.journal.info(self.pyre_family()) if channel is None else channel
        # everything below is a report
        if not channel:
            # so skip it if the channel is not active
            return

        # compute the margin
        margin = '  '*dent
        # render the shape
        shape = 'x'.join(map(str, mosaic.shape))
        # show me the region
        channel.line(f'{margin}region of interest: {region}')
        # the layout of the mosaic
        channel.line(f'{margin}mosaic: {shape} grid of tiles')
        # where the local store is
        channel.line(f"{margin}cache: '{self.cache.uri}'")
        # the name of the availability map
        channel.line(f'{margin}map: {availability.uri.name}')

        # mark the section
        channel.line(f'{margin}tiles:')

        necessary = ", ".join(tile.name for tile in mosaic)
        # show me the names of the needed tiles
        channel.line(f'{margin}    necessary: {necessary}')
        # mark the section
        channel.line(f'{margin}status:')

        # and now the status piles
        for status in self.availability:
            # build the list of names
            names = ", ".join(tile.name for tile in summary[status])
            # show me the code and the tiles
            channel.line(f'{margin}  {status.name:>11}: {names}')

        # all done
        return


    @isce.export
    def download(self, credentials=None, channel=None, dent=1, force=False, dry=False):
        """
        Retrieve the tiles necessary to cover the convex hull of my {region}

        Tile retrieval happens in parallel: the archive manager assembles a pool of workers
        that make independent attempts to contact the USGS data archive.
        """
        # build a mosaic that covers my region
        mosaic = self.mosaic(region=self.region, resolution=self.resolution)
        # check the status of its tiles
        summary = self.checkAvailability(mosaic=mosaic)
        # get my team manager
        team = self.team
        # adjust the desired number of workers
        team.size = self.pool
        # grab the node with my local tile archive
        cache = self.cache
        # and the tile availability map
        availability = self.availabilityMap

        # normalize the credentials
        credentials = credentials or (None, None)
        # prep the {urllib} support
        self.installCredentials(credentials=credentials)

        # otherwise, pull the crew type
        from .Retriever import Retriever
        # attach it
        team.crew = functools.partial(Retriever, map=availability, cache=cache)
        # assemble the work plan
        team.assemble(workplan=set(mosaic))
        # and fetch the tiles
        team.run()
        # all done
        return 0


    @isce.export
    def generate(self):
        """
        Generate the elevation model for the region of interest
        """
        self.info.log("NYI!")
        # all done
        return


    # implementation details
    @property
    def availabilityMap(self):
        """
        Fetch the SRTM tile availability map
        """
        # check whether i have done this before
        map = self._availabilityMap
        # and if i have
        if map is not None:
            # all done
            return map
        try:
            # otherwise, get the map
            map = self.loadAvailabilityMap()
        except Exception as error:
            print(error)
        # cache it
        self._availabilityMap = map
        # and return it
        return map


    @property
    def localstoreContents(self):
        """
        Build a set of the tile names that are present in the local cache
        """
        # check whether I have done this before
        contents = self._localstoreContents
        # and if i have
        if contents is not None:
            # all done
            return contents
        # otherwise, get my cache
        cache = self.cache
        # build the pattern for the filenames with relevant tiles
        pattern = self._filenamePattern.format(srtm=self)
        # get the contents
        contents = set(match.group('name') for node, match in cache.find(pattern=pattern))
        # save them
        self._localstoreContents = contents
        # and return them
        return contents


    def loadAvailabilityMap(self):
        """
        Retrieve the tile availability map
        """
        # grab the factory
        from .AvailabilityMap import AvailabilityMap as map
        # form the uri to the map
        uri = self.cache.uri / self._availabilityMapTemplate.format(srtm=self)
        # build it and return it
        return map(uri=uri)


    def checkAvailability(self, mosaic):
        """
        Update the status of all tiles that are available in the {localstore}
        """
        # the summary dictionary: map status codes to a list of tiles
        summary = collections.defaultdict(list)
        # get the availability map
        map = self.availabilityMap

        # go through each of my tiles
        for tile in mosaic:
            # check it against the availability map
            status = map.getTileAvailability(tile)
            # place it in the right pile
            summary[status].append(tile)
        # all done
        return summary


    def installCredentials(self, credentials):
        """
        Prepare to access the SRTM data archive
        """
        # unpack
        username, password = credentials
        # if i have both
        if username and password:
            # install them and return
            return self.urllibPrep(username=username, password=password)

        # otherwise, get my cache
        cache = self.cache
        # and attempt to
        try:
            # find the authentication database
            authdb = cache[self._authdb]
        # if not there
        except cache.NotFoundError:
            # build a description
            msg = "missing authentication database; please provide your Earthdata credentials"
            # and complain
            raise self.AuthenticationError(reason=msg)

        # now, try to
        try:
            # load its contents
            db = pickle.load(authdb.open(mode="rb"))
        # if this fails
        except EOFError:
            # build a description
            msg = "corrupt authentication database; please provide your Earthdata credentials"
            # and complain
            raise self.AuthenticationError(reason=msg)

        # if it's empty
        if not db:
            # build a description
            msg = "empty authentication database; please provide your Earthdata credentials"
            # and complain
            raise self.AuthenticationError(reason=msg)

        # if i have a specific username
        if username:
            # and it has a matching password
            try:
                # get it
                password = db[username]
            # if not
            except KeyError:
                # build a description
                msg = "no matching password for {!r}".format(username)
                # and complain
                raise self.AuthenticationError(reason=msg)
            # install and return
            return self.urllibPrep(username=username, password=password)

        # if the database contains more than one username
        if len(db) > 1:
            # build a description of what went wrong
            msg = "the authentication database contains multiple usernames; please pick one"
            # and complain
            raise self.AuthenticationError(reason=msg)

        # if there is only one, it must be the right one
        for username, password in db.items():
            # install and return
            return self.urllibPrep(username=username, password=password)

        # if we get this far, something went wrong...
        msg = "unknown error; please contact the developers"
        # and complain
        raise self.AuthenticationError(reason=msg)


    def urllibPrep(self, username, password):
        """
        Prepare the {urllib} machinery for the interaction with the SRTM data archive
        """
        # the location of the user profile
        realm = "http://urs.earthdata.nasa.gov"
        # make a cookie processor
        baker = urllib.request.HTTPCookieProcessor()
        # make a password manager
        pm = urllib.request.HTTPPasswordMgrWithPriorAuth()
        # install the user credentials
        pm.add_password(None, realm, username, password, is_authenticated=True)
        # build the authentication processor
        auth = urllib.request.HTTPBasicAuthHandler(pm)
        # chain them together
        opener = urllib.request.build_opener(auth, baker)
        # and install them
        urllib.request.install_opener(opener)
        # all done
        return


    # private data
    cache = None # the filesystem node with my local store

    # storage for my properties
    _availabilityMap = None
    _localstoreContents = None

    # constants
    # the authentication database file name
    _authdb = 'auth.db'
    # the availability map filename
    _availabilityMapTemplate = 'srtmgl{srtm.resolution}.map'
    # the pattern for matching valid tile names in the local store
    _filenamePattern = '(?P<name>(N|S)\d{{2}}(E|W)\d{{3}})\.SRTMGL{srtm.resolution}.hgt.zip'


# end of file
