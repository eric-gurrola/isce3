# -*- coding: utf-8 -*-
#
# michael a.g. aïvázis
# orthologue
# (c) 1998-2015 all rights reserved
#


# externals
import isce


# declaration
class About(isce.panel(), family='isce.actions.about'):
    """
    Display information about this application
    """


    # user configurable state
    root = isce.properties.str()
    root.tip = "specify the portion of the namespace to display"


    @isce.export(tip="the name of the app for configuration purposes")
    def name(self, plexus, **kwds):
        """
        Print the name of the app for configuration purposes
        """
        # show me
        plexus.info.log("{!r}".format(plexus.pyre_name) or "unknown")
        # all done
        return


    @isce.export(tip="the application home directory")
    def home(self, plexus, **kwds):
        """
        Print the application home directory
        """
        # show me
        plexus.info.log("{}".format(plexus.home))
        # all done
        return


    @isce.export(tip="the application installation directory")
    def prefix(self, plexus, **kwds):
        """
        Print the application installation directory
        """
        # show me
        plexus.info.log("{}".format(plexus.prefix))
        # all done
        return


    @isce.export(tip="the application configuration directory")
    def defaults(self, plexus, **kwds):
        """
        Print the application configuration directory
        """
        # show me
        plexus.info.log("{}".format(plexus.defaults))
        # all done
        return


    @isce.export(tip="print the version number")
    def version(self, plexus, **kwds):
        """
        Print the version of the isce package
        """
        # make some space
        plexus.info.log(isce.meta.header)
        # all done
        return


    @isce.export(tip="print the copyright note")
    def copyright(self, plexus, **kwds):
        """
        Print the copyright note of the isce package
        """
        # show the copyright note
        plexus.info.log(isce.meta.copyright)
        # all done
        return


    @isce.export(tip="print out the acknowledgments")
    def credits(self, plexus, **kwds):
        """
        Print out the license and terms of use of the isce package
        """
        # make some space
        plexus.info.log(isce.meta.acknowledgments)
        # all done
        return


    @isce.export(tip="print out the license and terms of use")
    def license(self, plexus, **kwds):
        """
        Print out the license and terms of use of the isce package
        """
        # make some space
        plexus.info.log(isce.meta.license)
        # all done
        return


    @isce.export(tip='dump the application configuration namespace')
    def nfs(self, plexus, **kwds):
        """
        Dump the application configuration namespace
        """
        # get the prefix
        prefix = self.root or 'isce'
        # show me
        plexus.pyre_nameserver.dump(prefix)
        # all done
        return


    @isce.export(tip='dump the application private filesystem')
    def pfs(self, plexus, **kwds):
        """
        Dump the application private filesystem
        """
        # build the report
        report = '\n'.join(plexus.pfs.dump())
        # sign in
        plexus.info.line('pfs:')
        # dump
        plexus.info.log(report)
        # all done
        return


    @isce.export(tip='dump the application virtual filesystem')
    def vfs(self, plexus, **kwds):
        """
        Dump the application virtual filesystem
        """
        # get the prefix
        prefix = self.root or '/isce'
        # build the report
        report = '\n'.join(plexus.vfs[prefix].dump())
        # sign in
        plexus.info.line('vfs: root={!r}'.format(prefix))
        # dump
        plexus.info.log(report)
        # all done
        return


# end of file
