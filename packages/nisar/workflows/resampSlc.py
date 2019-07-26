#!/usr/bin/env python3

import gdal
import isce3
from nisar.products.readers import SLC
from isce3.image.ResampSlc import ResampSlc

def cmdLineParse():
    """
    Command line parser.
    """
    parser = argparse.ArgumentParser(description="""
        Run resampSlc.""")

    # Required arguments
    parser.add_argument('product', type=str,
            help='Input HDF5 to be resampled.')
    parser.add_argument('reference', type=str,
            help='Reference HDF5 product.')
    parser.add_argument('frequency', type=str,
            help='Frequency of SLC.')
    parser.add_argument('polarization', type=str,
            help='Polarization of SLC.')

    # Optional arguments
    parser.add_argument('outdir', type=str, action='store', default='resampSlc',
            help='Output directory. Default: offsets.')
    parser.add_argument('offsetdir', type=str, action='store', default='offsets',
            help='Input offset directory. Default: offsets.')

    # Parse and return
    return parser.parse_args()


def main(opts):

    # prep SLC input
    productSlc = SLC(hdf5file=opts.product)
    referenceSlc = SLC(hdf5file=opts.reference)

    # get grids needed for resamp object instantiation
    productGrid = productSlc.getRadarGrid(opts.frequency)
    referenceGrid = referenceSlc.getRadarGrid(opts.frequency)

    # instantiate resamp object
    resamp = resampSlc(productGrid,
            productGrid.getDopplerCentroid(),
            productGrid.wavelength,
            referenceRadarGrid=referenceGrid,
            referenceWavelength=referenceGrid.wavelength)
    
    # Prepare input rasters
    inSlcRaster = productSlc.getSlcDataset(opts.frequency, opts.polarization)
    azOffsetRaster = isce3.io.raster(filename=os.path.join(opts.offsetdir, 'range.off'))
    rgOffsetRaster = isce3.io.raster(filename=os.path.join(opts.offsetdir, 'azimuth.off'))

    # Prepare output raster
    driver = gdal.GetDriverByName('ISCE')
    slcName = 'coreg_{}.slc'.format(opts.polarization)
    outds = driver.Create(os.path.join(outdir, slcName), rgOffsetRaster.width,
                          rgoffRaster.length, 1, gdal.GDT_CFloat32)
    outSlcRaster = isce3.io.raster('', dataset=outds)

    # Init output directory
    if not os.path.isdir(opts.outdir):
        os.mkdir(opts.outdir)

    # Run resamp
    resamp.resamp(inSlc=inSlcRaster,
            outSlc=outSlcRaster,
            rgoffRaster=rgOffsetRaster,
            azoffRaster=azOffsetRaster)


if __name__ == '__main__':
    opts = cmdLineParse()
    main(opts)

# end of file
