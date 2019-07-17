# -*- coding: utf-8 -*-

import os
import h5py
import pyre
import isce3
from ..protocols import ProductReader

class Base(pyre.component,
           family='nisar.productreader.base',
           implements=ProductReader):
    '''
    Base class for NISAR products.

    Contains common functionality that gets reused across products.
    '''
    _CFPath = pyre.properties.str(default='/')
    _CFPath.doc = 'Absolute path to scan for CF convention metadata'

    _RootPath = pyre.properties.str(default='/science/LSAR')
    _RootPath.doc = 'Absolute path to SAR data from L-SAR/S-SAR'

    _IdentificationPath = pyre.properties.str(default='identification')
    _IdentificationPath.doc = 'Absolute path ath to unique product identification information'

    _ProductType = pyre.properties.str(default='SLC')
    _ProductType.doc = 'The type of the product.'

    _MetadataPath = pyre.properties.str(default='metadata')
    _MetadataPath.doc = 'Relative path to metadata associated with standard product'

    _ProcessingInformation = pyre.properties.str(default='processingInformation')
    _ProcessingInformation.doc = 'Relative path to processing information associated with the product'

    _SwathPath = pyre.properties.str(default='swaths')
    _SwathPath.doc = 'Relative path to swaths associated with standard product'

    _GridPath = pyre.properties.str(default='grids')
    _GridPath.doc = 'Relative path to grids associated with standard product'

    productValidationType = pyre.properties.str(default='BASE')
    productValidationType.doc = 'Validation tag to compare identification information against to ensure that the right product type is being used.'

    def __init__(self, hdf5file='None', **kwds):
        '''
        Constructor.
        '''
        # Filename
        self.filename = hdf5file

        # Identification information
        self.identification = None

        # Polarization dictionary
        self.polarizations = {}

        self.data = {}
        self.__parse(self.filename)
    
    #@pyre.export
    def __parse(self, hdf5file):
        '''
        Parse the HDF5 file and populate ISCE data structures.
        '''

        self.filename = hdf5file
        self.populateIdentification()
        #For now, Needs to be an assertion check in the future
        self.identification.productType = self.productValidationType
        self.parsePolarizations()

    @pyre.export
    def getSwathMetadata(self, frequency='A'):
        '''
        Returns metadata corresponding to given frequency.
        '''
        swath = isce3.product.swath()
        with h5py.File(self.filename, 'r') as fid:
              swathGrp = fid[self.SwathPath]
              swath.loadFromH5(swathGrp, frequency)

        return swath

    @pyre.export
    def getRadarGrid(self, frequency='A'):
        '''
        Return radarGridParameters object
        '''
        swath = self.getSwathMetadata(frequency=frequency)
        return swath.getRadarGridParameters() 

    @pyre.export
    def getGridMetadata(self, frequency='A'):
        '''
        Returns metadata corresponding to given frequency.
        '''
        raise NotImplementedError

    @pyre.export
    def getOrbit(self):
        '''
        extracts orbit 
        '''

        #orbitPath = os.path.join(self._RootPath, self.productType, 
        #        self._MetadataPath, 'orbit')
        orbitPath = os.path.join(self.MetadataPath, 'orbit')

        orbit = isce3.core.orbit()
        with h5py.File(self.filename, 'r') as fid:
            orbitGrp = fid[orbitPath]
            orbit.loadFromH5(orbitGrp)
    
        return orbit

    @pyre.export
    def getDopplerCentroid(self, frequency='A'):
        '''
        Extract the Doppler centroid
        '''
        import numpy as np

        dopplerPath = os.path.join(self.ProcessingInformationPath, 
                                'parameters', 'frequency' + frequency, 
                                'dopplerCentroid') 

        zeroDopplerTimePath = os.path.join(self.ProcessingInformationPath,
                                            'parameters/zeroDopplerTime')

        slantRangePath = os.path.join(self.ProcessingInformationPath,
                                        'parameters/slantRange')
        # extract the native Doppler dataset
        with h5py.File(self.filename, 'r') as fid:
            doppler = fid[dopplerPath][:]
            zeroDopplerTime = fid[zeroDopplerTimePath][:]
            slantRange = fid[slantRangePath][:]

        dopplerCentroid = isce3.core.dopplerCentroid(x=slantRange, 
                                                    y=zeroDopplerTime, 
                                                    z=doppler)
        return dopplerCentroid

    @pyre.export
    def getZeroDopplerTime(self):
        '''
        Extract the azimuth time of the zero Doppler grid
        '''

        zeroDopplerTimePath = os.path.join(self.SwathPath, 
                                          'zeroDopplerTime')
        with h5py.File(self.filename, 'r') as fid:
            zeroDopplerTime = fid[zeroDopplerTimePath][:]

        return zeroDopplerTime

    @pyre.export
    def getSlantRange(self, frequency='A'):
        '''
        Extract the slant range of the zero Doppler grid
        '''

        slantRangePath = os.path.join(self.SwathPath,
                                    'frequency' + frequency, 'slantRange')

        with h5py.File(self.filename, 'r') as fid:
            slantRange = fid[slantRangePath][:]

        return slantRange


    def parsePolarizations(self):
        '''
        Parse HDF5 and identify polarization channels available for each frequency.
        '''
        from nisar.h5 import bytestring, extractWithIterator

        try:
            frequencyList = self.frequencies
        except:
            raise RuntimeError('Cannot determine list of available frequencies without parsing Product Identification')

        ###Determine if product has swaths / grids
        if self.productType.startswith('G'):
            folder = self.GridPath
        else:
            folder = self.SwathPath

        with h5py.File(self.filename, 'r') as fid:
            for freq in frequencyList:
                root = os.path.join(folder, 'frequency{0}'.format(freq))
                polList = extractWithIterator(fid[root], 'listOfPolarizations', bytestring,
                                              msg='Could not determine polarization for frequency{0}'.format(freq))
                self.polarizations[freq] = polList

        return

    def getSlcData(self, frequency, polarization):
        '''
        Return complex data from given frequency and polarization
        '''
        slcData = []
        with h5py.File(self.filename, 'r') as fid:
            root = os.path.join(folder, 'frequency{0}'.format(frequency), '{0}'.format(polarization))
            # key check performed in extractScalar(...)
            slcData = extractScalar(
                    fid[root], polarization, np.array,
                    msg=msg.format(frequency, polarization))

        return slcData

    def parseSlcData(self, frequency, polarization):
        '''
        Extract complex data from previously identified polarization channels
        '''
        try:
            frequencyList = self.frequencies
        except:
            raise RuntimeError('Cannot determine list of available frequencies without parsing Product Identification')

        nFandP = 0
        for frequency, polarizations in self.polarizations:
            nFandP += len(self.polarization(frequency))
        if nFandP == 0:
            raise RuntimeError('Cannot proceed with SLC extraction; No polarizations for any frequencies')
        
        # Single threaded read
        msg='Unable to extract to np.array SLC data of frequency {}, polarization {}'
        with h5py.File(self.filename, 'r') as fid:
            for frequency, polarizations in self.polarizations:
                self.slcData[frequency] = {}
                root = os.path.join(folder, 'frequency{0}'.format(frequency))
                for polarization in polarizations:

                    # investigate if using extract* is faster with its casting
                    # or using dataset ndarray methods
                    # https://stackoverflow.com/questions/46733052/read-hdf5-file-into-numpy-array
                    self.slcData[frequency][polarization] = extractScalar(
                            fid[root], polarization, np.array,
                            msg=msg.format(frequency, polarization))

        return


    @property
    def CFPath(self):
        return self._CFPath

    @property
    def RootPath(self):
        return self._RootPath

    @property
    def IdentificationPath(self):
        return os.path.join(self.RootPath, self._IdentificationPath)

    @property
    def ProductPath(self):
        return os.path.join(self.RootPath, self.productType)

    @property
    def MetadataPath(self):
        return os.path.join(self.ProductPath, self._MetadataPath)
    
    @property
    def ProcessingInformationPath(self):
        return os.path.join(self.MetadataPath, self._ProcessingInformation)

    @property
    def SwathPath(self):
        return os.path.join(self.ProductPath, self._SwathPath)

    @property
    def GridPath(self):
        return os.path.join(self.ProductPath, self._GridPath)

    @property
    def productType(self):
        return self.identification.productType

    def populateIdentification(self):
        '''
        Read in the Identification information and assert identity.
        '''
        import h5py
        from .Identification import Identification

        with h5py.File(self.filename, 'r') as fileID:
            h5grp = fileID[self.IdentificationPath]
            self.identification = Identification(h5grp)

    @property
    def frequencies(self):
        '''
        Return list of frequencies in the product.
        '''
        return self.identification.listOfFrequencies

    @staticmethod
    def validate(self, hdf5file):
        '''
        Validate a given HDF5 file.
        '''
        raise NotImplementedError

    def computeBoundingBox(self, epsg=4326):
        '''
        Compute the bounding box as a polygon in given projection system.
        '''
        raise NotImplementedError

# end of file
