//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Piyush Agram
// Copyright 2019


#include "cpl_port.h"
#include "IH5Dataset.h"

#include "cpl_config.h"
#include "cpl_conv.h"
#include "cpl_error.h"
#include "cpl_minixml.h"
#include "cpl_progress.h"
#include "cpl_string.h"
#include "cpl_vsi.h"
#include "gdal.h"
#include "gdal_frmts.h"
#include "gdal_priv.h"
#include <vector>

//Exposing some hidden HDF5 functionality to allow for interfacing
namespace H5
{
    void f_DataSet_setId(H5::DataSet *dset,hid_t new_id );
    void f_DataType_setId(H5::DataType *dtype, hid_t new_id);
}


namespace isce{
namespace io{

/************************************************************************/
/*                           IH5RasterBand()                            */
/************************************************************************/

IH5RasterBand::IH5RasterBand( IH5Dataset *ds, int bandIn,
                              GDALDataType eTypeIn) :
    GDALPamRasterBand(FALSE),
    bNoDataSet(FALSE),
    dfNoData(0.0)
{
    //Set parent dataset pointer
    poDS = ds;

    //Access is passed in as an argument
    eAccess = ds->eAccess;

    //Set band number 
    nBand = bandIn;

    //This is passed as an argument
    //Determined at dataset level
    eDataType = eTypeIn;

    //use ds->getChunks() to get these numbers when function becomes available.
    int indexOffset = (ds->ndims == 3) ? 1: 0;

    nBlockYSize = ds->chunks[indexOffset];
    nBlockXSize = ds->chunks[indexOffset+1];

}

/************************************************************************/
/*                           ~IH5RasterBand()                           */
/************************************************************************/

IH5RasterBand::~IH5RasterBand()
{
    IH5RasterBand::FlushCache();
}

/************************************************************************/
/*                             IReadBlock()                             */
/************************************************************************/

CPLErr IH5RasterBand::IReadBlock( int nBlockXOff,
                                  int nBlockYOff,
                                  void * pImage )
{

    //Determine native data type to use 
    IH5Dataset *poGDS = static_cast<IH5Dataset *>(poDS);
    H5::DataType readType = poGDS->nativeType;

    if ( poGDS->eAccess == GA_Update )
    {
        memset(pImage, 0,
               nBlockXSize * nBlockYSize * GDALGetDataTypeSize(eDataType)/8);
        return CE_None;
    }

    int dims = poGDS->_dataset->getRank();
    int starts[3];
    int counts[3];
    int offset = 0;
   
    //If 3D dataset is being used
    if (dims == 3)
    {
        starts[0] = nBand-1;
        counts[0] = 1;
        offset = 1;
    }

    starts[offset] = nBlockYOff * nBlockYSize;
    starts[offset+1] = nBlockXOff * nBlockXSize;

    //Account for partial blocks
    counts[offset] = std::min(nBlockYSize, poGDS->GetRasterYSize() - starts[offset]);
    counts[offset+1] = std::min(nBlockXSize, poGDS->GetRasterXSize() - starts[offset+1]);
  
    CPLDebug("GDAL_IH5", "%lld Read, band=%d, starts=(%d,%d), counts=(%d,%d)", 
            poGDS->_dataset->getId(), nBand, 
            starts[offset], starts[offset+1],
            counts[offset], counts[offset+1]);
    H5::DataSpace dspace = poGDS->_dataset->getDataSpace(starts, counts, nullptr);
    if (!H5::IdComponent::isValid(dspace.getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Failure to get data space in Read Block");
        return CE_Failure;
    }


    //Set up appropriate memory space
    hsize_t blockdims[2];
    blockdims[0] = nBlockYSize;
    blockdims[1] = nBlockXSize;

    hsize_t blockcounts[2];
    blockcounts[0] = counts[offset];
    blockcounts[1] = counts[offset+1];

    hsize_t blockoffsets[2];
    blockoffsets[0] = 0;
    blockoffsets[1] = 0;

    H5::DataSpace mspace(2, blockdims);
    mspace.selectHyperslab( H5S_SELECT_SET, blockcounts, blockoffsets);
    if (!H5::IdComponent::isValid(dspace.getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Failure to get memory space in Read Block");
        return CE_Failure;
    }
     
    poGDS->_dataset->H5::DataSet::read(pImage, readType, mspace, dspace);
    if (!H5::IdComponent::isValid(poGDS->_dataset->getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Failure to read data in Read Block");
        return CE_Failure;
    }

    if (H5::IdComponent::isValid(dspace.getId()))
        dspace.close();

    if (H5::IdComponent::isValid(mspace.getId()))
        mspace.close();
    
    return CE_None;
}

/************************************************************************/
/*                            IWriteBlock()                             */
/************************************************************************/

CPLErr IH5RasterBand::IWriteBlock( int nBlockXOff,
                                   int nBlockYOff,
                                   void * pImage )
{

    //Determine native data type to use
    IH5Dataset *poGDS = static_cast<IH5Dataset *>(poDS);
    H5::DataType writeType = poGDS->nativeType;

    int dims = poGDS->_dataset->getRank();
    int starts[3];
    int counts[3];
    int offset = 0;

    //If 3D dataset is being used
    if (dims == 3)
    {
        starts[0] = nBand-1;
        counts[0] = 1;
        offset = 1;
    }

    starts[offset] = nBlockYOff * nBlockYSize;
    starts[offset+1] = nBlockXOff * nBlockXSize;

    //Account for partial blocks
    counts[offset] = std::min(nBlockYSize, 
                        poGDS->GetRasterYSize() - starts[offset]);
    counts[offset+1] = std::min(nBlockXSize, 
                        poGDS->GetRasterXSize() - starts[offset+1]);

    CPLDebug("GDAL_IH5", "%lld Write, band=%d, starts=(%d,%d), counts=(%d,%d)", 
            poGDS->_dataset->getId(), nBand, 
            starts[offset], starts[offset+1],
            counts[offset], counts[offset+1]);

    H5::DataSpace dspace = poGDS->_dataset->getDataSpace(starts, counts, nullptr);
    if (!H5::IdComponent::isValid(dspace.getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Failure to get data space in Write Block");
        return CE_Failure;
    } 

    //Set up appropriate memory space
    hsize_t blockdims[2];
    blockdims[0] = nBlockYSize;
    blockdims[1] = nBlockXSize;

    hsize_t blockcounts[2];
    blockcounts[0] = counts[offset];
    blockcounts[1] = counts[offset+1];

    hsize_t blockoffsets[2];
    blockoffsets[0] = 0;
    blockoffsets[1] = 0;

    H5::DataSpace mspace(2, blockdims);
    mspace.selectHyperslab( H5S_SELECT_SET, blockcounts, 
                blockoffsets); 
    if (!H5::IdComponent::isValid(mspace.getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Failure to get memory space in Write Block");
        return CE_Failure;
    }

    poGDS->_dataset->H5::DataSet::write(pImage, writeType, 
            mspace, dspace);
    if (!H5::IdComponent::isValid(poGDS->_dataset->getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Failure to write data in Write Block");
        return CE_Failure;
    }
    if (H5::IdComponent::isValid(dspace.getId()))
        dspace.close();

    if (H5::IdComponent::isValid(mspace.getId()))
        mspace.close();

    return CE_None;
}

/************************************************************************/
/*                            GetNoDataValue()                          */
/************************************************************************/
double IH5RasterBand::GetNoDataValue( int *pbSuccess )

{
    if( pbSuccess )
        *pbSuccess = bNoDataSet;

    if( bNoDataSet )
        return dfNoData;

    return 0.0;
}

/************************************************************************/
/*                            SetNoDataValue()                          */
/************************************************************************/
CPLErr IH5RasterBand::SetNoDataValue( double dfNewValue )
{
    dfNoData = dfNewValue;
    bNoDataSet = TRUE;

    return CE_None;
}

/************************************************************************/
/*                            IH5Dataset()                              */
/************************************************************************/

IH5Dataset::IH5Dataset(const hid_t &inputds, GDALAccess eAccessIn ):
    dimensions(), chunks()
{
    bGeoTransformSet = false;
    pszProjection = nullptr;
    nGCPCount = 0;
    pasGCPList = nullptr;
    ndims = 0;
   
    adfGeoTransform[0] = 0.0;
    adfGeoTransform[1] = 1.0;
    adfGeoTransform[2] = 0.0;
    adfGeoTransform[3] = 0.0;
    adfGeoTransform[4] = 0.0;
    adfGeoTransform[5] = -1.0;
   
    if (!H5::IdComponent::isValid(inputds))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Not a valid HDF5 ID to create IH5Dataset from");
        return;
    }
    CPLDebug("GDAL_IH5", "Input ID = %lld", inputds);
    eAccess = eAccessIn;
    _dataset = new isce::io::IDataSet(inputds);
    CPLDebug("GDAL_IH5", "Extracted ID = %lld", _dataset->getId());
    if (!H5::IdComponent::isValid(_dataset->getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Could not convert HDF5 ID to a IDataSet");
        return;
    }

    if (populateFromDataset())
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "Failed to create a IH5Dataset from provided object");
        return;
    }
}

/************************************************************************/
/*                            ~IH5Dataset()                             */
/************************************************************************/

IH5Dataset::~IH5Dataset()

{
    FlushCache();

    //Close the C++ types
    if (H5::IdComponent::isValid(nativeType.getId()))
        nativeType.close();

    if (H5::IdComponent::isValid(actualType.getId()))
        actualType.close();

    if (H5::IdComponent::isValid(_dataset->getId()))
        _dataset->close();

    //Free the memory
    delete _dataset;

    CPLFree( pszProjection );

    if( nGCPCount > 0 )
    {
        for( int i = 0; i < nGCPCount; i++ )
        {
            CPLFree(pasGCPList[i].pszId);
            CPLFree(pasGCPList[i].pszInfo);
        }
        CPLFree(pasGCPList);
    }
}

// Thought this might be handy to pass back to the application
void * IH5Dataset::GetInternalHandle(const char *)
{
     return _dataset;
}

#if GDAL_VERSION_MAJOR == 2
const char *IH5Dataset::GetProjectionRef()
#elif GDAL_VERSION_MAJOR == 3
const char *IH5Dataset::_GetProjectionRef()
#endif
{
    if( pszProjection == nullptr )
        return "";

    return pszProjection;
}

/************************************************************************/
/*                           SetProjection()                            */
/************************************************************************/

#if GDAL_VERSION_MAJOR == 2
CPLErr IH5Dataset::SetProjection( const char *pszProjectionIn )
#elif GDAL_VERSION_MAJOR == 3
CPLErr IH5Dataset::_SetProjection( const char *pszProjectionIn)
#endif
{
    CPLFree( pszProjection );
    pszProjection = CPLStrdup( pszProjectionIn );

    return CE_None;
}

/************************************************************************/
/*                          GetGeoTransform()                           */
/************************************************************************/

CPLErr IH5Dataset::GetGeoTransform( double *padfGeoTransform )
{
    memcpy( padfGeoTransform, adfGeoTransform, sizeof(double) * 6 );
    if( bGeoTransformSet )
        return CE_None;

    return CE_Failure;

}

/************************************************************************/
/*                          SetGeoTransform()                           */
/************************************************************************/

CPLErr IH5Dataset::SetGeoTransform( double *padfGeoTransform )

{
    memcpy( adfGeoTransform, padfGeoTransform, sizeof(double) * 6 );
    bGeoTransformSet = TRUE;

    return CE_None;
}

/************************************************************************/
/*                            GetGCPCount()                             */
/************************************************************************/

int IH5Dataset::GetGCPCount()

{
    return nGCPCount;
    //Get dimensions and fill the sizes
}

/************************************************************************/
/*                          GetGCPProjection()                          */
/************************************************************************/
#if GDAL_VERSION_MAJOR == 2
const char *IH5Dataset::GetGCPProjection()
#elif GDAL_VERSION_MAJOR == 3
const char *IH5Dataset::_GetGCPProjection()
#endif
{
    return pszGCPProjection;
}

/************************************************************************/
/*                              GetGCPs()                               */
/************************************************************************/

const GDAL_GCP *IH5Dataset::GetGCPs()
{
    return pasGCPList;
}

/************************************************************************/
/*                              SetGCPs()                               */
/************************************************************************/
#if GDAL_VERSION_MAJOR == 2
CPLErr IH5Dataset::SetGCPs( int nNewCount, const GDAL_GCP *pasNewGCPList,
                            const char *inpGCPProjection )
#elif GDAL_VERSION_MAJOR == 3
CPLErr IH5Dataset::_SetGCPs( int nNewCount, const GDAL_GCP *pasNewGCPList,
                            const char *inpGCPProjection )
#endif
{
    GDALDeinitGCPs( nGCPCount, pasGCPList );
    CPLFree( pasGCPList );

    if( inpGCPProjection == nullptr )
        pszGCPProjection = CPLStrdup("");
    else
        pszGCPProjection = inpGCPProjection;

    nGCPCount = nNewCount;
    pasGCPList = GDALDuplicateGCPs( nGCPCount, pasNewGCPList );

    return CE_None;
}

/************************************************************************/
/*                  IH5GetNativeType()                                  */
/************************************************************************/
H5::DataType IH5GetNativeType(H5::DataType &intype)
{
    H5::DataType nativeType;
     
    //Resort to C API to get native type.
    //Does not seem to have been exposed in C++ API.
    //TODO - moving this function to IH5 API.
    hid_t native_c = H5Tget_native_type(intype.getId(), H5T_DIR_ASCEND);
    if (native_c < 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "Could not determine native data type for IH5Dataset");
    }
    else
    {
        nativeType = H5::DataType(native_c);
    }
    return nativeType;
}

/************************************************************************/
/*                 populateFromDataset()                                */
/************************************************************************/

CPLErr IH5Dataset::populateFromDataset()
{
    //Get dimensions
    ndims = _dataset->getRank();
    if (!H5::IdComponent::isValid(_dataset->getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "IH5Dataset->getRank() failure");
        return CE_Failure;
    }
    if ((ndims != 2) && (ndims != 3))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "IH5Dataset can only handle arrays of dimension 2 or 3");
        return CE_Failure;
    }

    //Determine offset to Y-axis
    int axisOffset = (ndims == 3)?1:0;
    CPLDebug("GDAL_IH5", "%lld: Number of axes = %d", _dataset->getId(), ndims);

    //Get dimensions - row major layout
    std::vector<int> dims = _dataset->getDimensions();
    if (!H5::IdComponent::isValid(_dataset->getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "IH5Dataset->getDimensions() failure");
        return CE_Failure;
    }

    for(int ii=0; ii < ndims; ii++)
        dimensions[ii] = dims[ii];

    nRasterYSize = dimensions[axisOffset];
    nRasterXSize = dimensions[axisOffset+1];

    nBands = (ndims == 3) ? dimensions[0] : 1;
    if (ndims == 2)
        CPLDebug("GDAL_IH5", "%lld: %d L x %d P", _dataset->getId(), dimensions[axisOffset], dimensions[axisOffset+1]);
    else
        CPLDebug("GDAL_IH5", "%lld: %d H x %d L x %d P", _dataset->getId(), nBands, dimensions[axisOffset], dimensions[axisOffset+1]);


    auto chunkSize = _dataset->getChunkSize();
    if (!H5::IdComponent::isValid(_dataset->getId()))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "IH5Dataset->getChunkSize() failure");
        return CE_Failure;
    }
    chunks[0] = 1;
    chunks[axisOffset] = (chunkSize[axisOffset] == 0) ? 1 : chunkSize[axisOffset];
    chunks[axisOffset+1] = (chunkSize[axisOffset] == 0) ? nRasterXSize : chunkSize[axisOffset+1];

    //Start detecting data type
    actualType = _dataset->getDataType();
    if (! (H5::IdComponent::isValid(actualType.getId()) &&
            H5::IdComponent::isValid(_dataset->getId())))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "IH5Dataset actual type detection failure");
        return CE_Failure;
    }

    H5::f_DataType_setId(&nativeType, IH5GetNativeType(actualType).getId());
    if (! (H5::IdComponent::isValid(actualType.getId()) &&
            H5::IdComponent::isValid(nativeType.getId())))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "IH5Dataset native type detection failure");
        return CE_Failure;
    }

    //Determine equivalent GDAL Datatype
    GDALDataType eType = GDT_Unknown;
    //Check for native types first
    if (nativeType.getClass() != H5T_COMPOUND )
    {

        if( nativeType == H5::PredType::NATIVE_CHAR )
            eType =  GDT_Byte;
        else if( nativeType == H5::PredType::NATIVE_SCHAR )
            eType =  GDT_Byte;
        else if( nativeType == H5::PredType::NATIVE_UCHAR )
            eType =  GDT_Byte;
        else if( nativeType == H5::PredType::NATIVE_SHORT )
            eType =  GDT_Int16;
        else if( nativeType == H5::PredType::NATIVE_USHORT )
            eType = GDT_UInt16;
        else if( nativeType == H5::PredType::NATIVE_INT )
            eType = GDT_Int32;
        else if( nativeType == H5::PredType::NATIVE_UINT )
            eType = GDT_UInt32;
        else if( nativeType == H5::PredType::NATIVE_LONG )
        {
            eType = GDT_Int32;
        }
        else if( nativeType == H5::PredType::NATIVE_ULONG )
        {
            eType = GDT_UInt32;
        }
        else if( nativeType == H5::PredType::NATIVE_FLOAT )
            eType = GDT_Float32;
        else if( nativeType == H5::PredType::NATIVE_DOUBLE )
            eType = GDT_Float64;
    }
    else if (nativeType.getClass() ==  H5T_COMPOUND) 
    {
        //Get compound data type
        H5::CompType compoundType(*_dataset);
        if (! (H5::IdComponent::isValid(compoundType.getId()) &&
                H5::IdComponent::isValid(_dataset->getId())))
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                "IH5Dataset compound type detection failure");
            return CE_Failure;
        }

        //For complex the compound type must contain 2 elements
        if ( compoundType.getNmembers() == 2 )
        {
            //For complex the native types of both elements should be the same
            H5::DataType firstType = compoundType.getMemberDataType(0);
            H5::DataType secondType = compoundType.getMemberDataType(1);

            if (firstType == secondType)
            {

                H5::DataType nativeFirstType;
                f_DataType_setId(&nativeFirstType, IH5GetNativeType(firstType).getId());

                if ( nativeFirstType == H5::PredType::NATIVE_FLOAT )
                {
                    eType = GDT_CFloat32;
                }
                else if ( nativeFirstType == H5::PredType::NATIVE_DOUBLE ) 
                {
                    eType = GDT_CFloat64;
                }
                //TODO - Support complex int as well for other sensors
                /*else if ( nativeFirstType == H5::PredType::NATIVE_SHORT )
                {,
                    eDataType = GDT_CInt16;
                }
                else if ( nativeFirstType == H5::PredType::NATIVE_INT )
                {
                    eDataType = GDT_CInt32;
                }
                else ( nativeFirstType == H5::PredType::NATIVE_LONG )
                {
                    eDataType = GDT_CInt32;
                }*/
                nativeFirstType.close();
            }
            //Close the data type
            firstType.close();
            secondType.close();
        }
        compoundType.close();
    }

    //If we havent been able to parse the type so far
    //Return with failure status
    if (eType == GDT_Unknown)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                "Could not determine datatype by parsing native types");
        actualType.close();
        nativeType.close();
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Create band information objects.                                */
/* -------------------------------------------------------------------- */
    for( int iBand = 0; iBand < nBands; iBand++ )
    {
        IH5RasterBand *poNewBand = new IH5RasterBand( this, iBand+1, eType);
        SetBand( iBand+1, poNewBand );
    }

    return CE_None;
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

GDALDataset *IH5Dataset::Open( GDALOpenInfo * poOpenInfo )

{
/* -------------------------------------------------------------------- */
/*      Do we have the special filename signature for MEM format        */
/*      description strings?                                            */
/* -------------------------------------------------------------------- */
    if( !Identify(poOpenInfo))
        return nullptr;

    char **papszOptions
        = CSLTokenizeStringComplex(poOpenInfo->pszFilename+6, ",",
                                   TRUE, FALSE );

/* -------------------------------------------------------------------- */
/*      Verify we have all required fields                              */
/* -------------------------------------------------------------------- */
    if( CSLFetchNameValue( papszOptions, "ID" ) == nullptr )
    {
        CPLError(
            CE_Failure, CPLE_AppDefined,
            "Missing required field ID.  "
            "Unable to access IH5 object." );

        CSLDestroy( papszOptions );
        return nullptr;
    }
/* -------------------------------------------------------------------- */
/*      Create the new MEMDataset object.                               */
/* -------------------------------------------------------------------- */
    const hid_t h5datasetid = atoll(CSLFetchNameValue(papszOptions, "ID"));
    GDALAccess eacc = poOpenInfo->eAccess;

    IH5Dataset *poDS = new IH5Dataset(h5datasetid, eacc);

    poDS->SetDescription(poOpenInfo->pszFilename);
    return poDS;

}

/************************************************************************/
/*                     Identify()                                    */
/************************************************************************/

int IH5Dataset::Identify( GDALOpenInfo * poOpenInfo )
{
    int result = 0;
    if(STARTS_WITH(poOpenInfo->pszFilename, "IH5:::") &&
            poOpenInfo->fpL == nullptr)
        result = 1;

    return result;
}


/************************************************************************/
/*                        GDALRegister_IH5()                            */
/************************************************************************/
void GDALRegister_IH5()
{
    if( !GDAL_CHECK_VERSION("IH5 driver") )
        return;

    if( GDALGetDriverByName("IH5") != nullptr )
        return;

    GDALDriver *poDriver = new GDALDriver();

    poDriver->SetDescription("IH5");
    poDriver->SetMetadataItem(GDAL_DCAP_RASTER, "YES");
    poDriver->SetMetadataItem(GDAL_DMD_LONGNAME, "ISCE IH5 Raster");

    poDriver->pfnOpen = IH5Dataset::Open;
    poDriver->pfnIdentify = IH5Dataset::Identify;

    GetGDALDriverManager()->RegisterDriver(poDriver);
}

}
}
