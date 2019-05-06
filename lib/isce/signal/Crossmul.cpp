// -*- C++ -*-
// -*- coding: utf-8 -*-
//
// Author: Heresh Fattahi, Bryan Riel
// Copyright 2018-
//

#include "Crossmul.h"

// Utility function to get number of OpenMP threads
// (gcc sometimes has problems with omp_get_num_threads)
size_t omp_thread_count() {
    size_t n = 0;
    #pragma omp parallel reduction(+:n)
    n += 1;
    return n;
}

/*
isce::signal::Crossmul::
Crossmul(const isce::product::Product& referenceSlcProduct,
         const isce::product::Product& secondarySlcProduct,
         isce::product::Product& outputInterferogramProduct)
*/

/**
* @param[in] referenceSLC Raster object of refernce SLC
* @param[in] secondarySLC Raster object of secondary SLC
* @param[out] interferogram Raster object of output interferogram
*/
void isce::signal::Crossmul::
crossmul(isce::io::Raster& referenceSLC,
        isce::io::Raster& secondarySLC,
        isce::io::Raster& interferogram)
{

    _computeCoherence = false;
    _doCommonRangebandFilter = false;

    isce::io::Raster rngOffsetRaster("/vsimem/dummy", 1, 1, 1, GDT_Float32, "ENVI");
    isce::io::Raster coherence("/vsimem/dummyCoh", 1, 1, 1, GDT_Float32, "ENVI");

    crossmul(referenceSLC,
            secondarySLC,
            rngOffsetRaster,
            interferogram,
            coherence);

}


/**
* @param[in] referenceSLC Raster object of refernce SLC
* @param[in] secondarySLC Raster object of secondary SLC
* @param[out] interferogram Raster object of output interferogram
* @param[out] coherence Raster object of output coherence
*/
void isce::signal::Crossmul::
crossmul(isce::io::Raster& referenceSLC,
        isce::io::Raster& secondarySLC,
        isce::io::Raster& interferogram,
        isce::io::Raster& coherence)
{

    _doCommonRangebandFilter = false;
    isce::io::Raster rngOffsetRaster("/vsimem/dummy", 1, 1, 1, GDT_CFloat32, "ENVI");
    crossmul(referenceSLC, 
            secondarySLC,
            rngOffsetRaster,
            interferogram,
            coherence);

}

/**
* @param[in] referenceSLC Raster object of refernce SLC
* @param[in] secondarySLC Raster object of secondary SLC
* @param[in] rngOffsetRaster Raster object of range offsets between reference and secondary SLCs
* @param[out] interferogram Raster object of output interferogram
* @param[out] coherence Raster object of output coherence
*/
void isce::signal::Crossmul::
crossmul(isce::io::Raster& referenceSLC,
        isce::io::Raster& secondarySLC,
        isce::io::Raster& rngOffsetRaster,
        isce::io::Raster& interferogram,
        isce::io::Raster& coherenceRaster)
{

    // Create reusable pyre::journal channels
    pyre::journal::warning_t warning("isce.geometry.Topo");
    pyre::journal::info_t info("isce.geometry.Topo");

    size_t nrows = referenceSLC.length();
    size_t ncols = referenceSLC.width();
    size_t nthreads = omp_thread_count();

    //signal object for refSlc
    isce::signal::Signal<float> refSignal(nthreads);

    //signal object for secSlc
    isce::signal::Signal<float> secSignal(nthreads);

    // instantiate Looks used for multi-looking the interferogram
    isce::signal::Looks<float> looksObj;

    // setting the parameters of the multi-looking oject
    if (_doMultiLook){
        // Making sure that the number of rows in each block (blockRows) 
        // to be an integer number of azimuth looks.
        blockRows = (blockRows/_azimuthLooks)*_azimuthLooks;
    }
    size_t blockRowsMultiLooked = blockRows/_azimuthLooks;
    size_t ncolsMultiLooked = ncols/_rangeLooks;
    looksObj.nrows(blockRows);
    looksObj.ncols(ncols);
    looksObj.rowsLooks(_azimuthLooks);
    looksObj.colsLooks(_rangeLooks);
    looksObj.nrowsLooked(blockRowsMultiLooked);
    looksObj.ncolsLooked(ncolsMultiLooked);
    
    // Compute FFT size (power of 2)
    size_t nfft;
    refSignal.nextPowerOfTwo(ncols, nfft);

    // number of blocks to process
    size_t nblocks = nrows / blockRows;
    if (nblocks == 0) {
        nblocks = 1;
    } else if (nrows % (nblocks * blockRows) != 0) {
        nblocks += 1;
    }

    // storage for a block of reference SLC data
    std::valarray<std::complex<float>> refSlc(nfft*blockRows);

    // storage for a block of secondary SLC data
    std::valarray<std::complex<float>> secSlc(nfft*blockRows);

    // storage for a block of range offsets
    std::valarray<double> rngOffset(ncols*blockRows);

    // storage for a simulated interferogram which its phase is the 
    // interferometric phase due to the imaging geometry:
    // phase = (4*PI/wavelength)*(rngOffset) 
    std::valarray<std::complex<float>> geometryIfgram(nfft*blockRows);

    // complex conjugate of geometryIfgram
    std::valarray<std::complex<float>> geometryIfgramConj(nfft*blockRows);

    // storage for spectrum of the block of data in reference SLC
    std::valarray<std::complex<float>> refSpectrum(nfft*blockRows);

    // storage for spectrum of the block of data in secondary SLC
    std::valarray<std::complex<float>> secSpectrum(nfft*blockRows);

    // upsampled spectrum of the block of reference SLC
    std::valarray<std::complex<float>> refSpectrumUpsampled(oversample*nfft*blockRows);

    // upsampled spectrum of the block of secondary SLC
    std::valarray<std::complex<float>> secSpectrumUpsampled(oversample*nfft*blockRows);

    // upsampled block of reference SLC 
    std::valarray<std::complex<float>> refSlcUpsampled(oversample*nfft*blockRows);

    // upsampled block of secondary SLC
    std::valarray<std::complex<float>> secSlcUpsampled(oversample*nfft*blockRows);

    // upsampled interferogram
    std::valarray<std::complex<float>> ifgramUpsampled(oversample*ncols*blockRows);

    // full resolution interferogram
    std::valarray<std::complex<float>> ifgram(ncols*blockRows);

    // multi-looked interferogram
    std::valarray<std::complex<float>> ifgramMultiLooked(ncolsMultiLooked*blockRowsMultiLooked);

    // multi-looked amplitude of reference SLC
    std::valarray<float> refAmplitudeLooked(ncolsMultiLooked*blockRowsMultiLooked);

    // multi-looked amplitude of secondary SLC
    std::valarray<float> secAmplitudeLooked(ncolsMultiLooked*blockRowsMultiLooked);

    // coherence for multi-looked interferogram
    std::valarray<float> coherence(ncolsMultiLooked*blockRowsMultiLooked);

    // make forward and inverse fft plans for the reference SLC 
    refSignal.forwardRangeFFT(refSlc, refSpectrum, nfft, blockRows);
    refSignal.inverseRangeFFT(refSpectrumUpsampled, refSlcUpsampled, nfft*oversample, blockRows);

    // make forward and inverse fft plans for the secondary SLC
    secSignal.forwardRangeFFT(secSlc, secSpectrum, nfft, blockRows);
    secSignal.inverseRangeFFT(secSpectrumUpsampled, secSlcUpsampled, nfft*oversample, blockRows);

    // looking down the upsampled interferogram may shift the samples by a fraction of a pixel
    // depending on the oversample factor. predicting the impact of the shift in frequency domain 
    // which is a linear phase allows to account for it during the upsampling process
    std::valarray<std::complex<float>> shiftImpact(oversample*nfft*blockRows);
    lookdownShiftImpact(oversample,  nfft, 
                        blockRows, shiftImpact);

    //filter objects which will be used for azimuth and range common band filtering
    isce::signal::Filter<float> azimuthFilter;
    isce::signal::Filter<float> rangeFilter;

    std::valarray<double> rangeFrequencies(nfft);
    rangeFilter.fftfreq(1.0/_rangeSamplingFrequency, rangeFrequencies);

    rangeFilter.initiateRangeFilter(refSlc, refSpectrum, nfft, blockRows);

    // storage for azimuth spectrum used by filter
    std::valarray<std::complex<float>> refAzimuthSpectrum(nfft*blockRows);

    if (_doCommonAzimuthbandFilter){
        // construct azimuth common band filter for a block of data
        azimuthFilter.constructAzimuthCommonbandFilter(_refDoppler, 
                                            _secDoppler, 
                                            _commonAzimuthBandwidth,
                                            _prf, 
                                            _beta,
                                            refSlc, refAzimuthSpectrum,
                                            nfft, blockRows);
    }

    // loop over all blocks
    std::cout << "nblocks : " << nblocks << std::endl;

    for (size_t block = 0; block < nblocks; ++block) {
        std::cout << "block: " << block << std::endl;       
        // start row for this block
        size_t rowStart;
        rowStart = block * blockRows;
        
        //number of lines of data in this block. blockRowsData<= blockRows
        //Note that blockRows is fixed number of lines
        //blockRowsData might be less than or equal to blockRows.
        //e.g. if nrows = 512, and blockRows = 100, then 
        //blockRowsData for last block will be 12
        size_t blockRowsData;
        if ((rowStart + blockRows) > nrows) {
            blockRowsData = nrows - rowStart;
        } else {
            blockRowsData = blockRows;
        }

        // fill the valarray with zero before getting the block of the data
        refSlc = 0;
        secSlc = 0;
        ifgramUpsampled = 0;
        ifgram = 0;

        // get a block of reference and secondary SLC data
        // and a block of range offsets
        // This will change once we have the functionality to 
        // get a block of data directly in to a slice
        std::valarray<std::complex<float>> dataLine(ncols);
        for (size_t line = 0; line < blockRowsData; ++line){
            referenceSLC.getLine(dataLine, rowStart + line);
            refSlc[std::slice(line*nfft, ncols, 1)] = dataLine;
            secondarySLC.getLine(dataLine, rowStart + line);
            secSlc[std::slice(line*nfft, ncols, 1)] = dataLine;
        }
        //referenceSLC.getBlock(refSlc, 0, rowStart, ncols, blockRowsData);
        //secondarySLC.getBlock(secSlc, 0, rowStart, ncols, blockRowsData);
   
        //commaon azimuth band-pass filter the reference and secondary SLCs
        if (_doCommonAzimuthbandFilter){
            azimuthFilter.filter(refSlc, refAzimuthSpectrum);
            azimuthFilter.filter(secSlc, refAzimuthSpectrum);
        }

        // common range band-pass filtering
        if (_doCommonRangebandFilter) {

            // Some diagnostic messages to make sure everything has been configured
            std::cout << " - range pixel spacing: " << _rangePixelSpacing << std::endl;
            std::cout << " - wavelength: " << _wavelength << std::endl;

            // Read range offsets
            std::valarray<double> offsetLine(ncols);
            for (size_t line = 0; line < blockRowsData; ++line){
                rngOffsetRaster.getLine(offsetLine, rowStart + line);
                rngOffset[std::slice(line*ncols, ncols, 1)] = offsetLine;
            }

            #pragma omp parallel for
            for (size_t line = 0; line < blockRowsData; ++line){
                for (size_t col = 0; col < ncols; ++col){
                    double phase = 4.0*M_PI*_rangePixelSpacing*rngOffset[line*ncols+col]/_wavelength;
                    geometryIfgram[line*nfft + col] = std::complex<float> (std::cos(phase), std::sin(phase));
                    geometryIfgramConj[line*nfft + col] = std::complex<float> (std::cos(phase), 
                                                                            -1.0*std::sin(phase));

                }
            }

            // Forward FFT to compute topo-dependent spectrum
            refSignal.forward(geometryIfgramConj, refSpectrum);
            refSignal.forward(geometryIfgram, secSpectrum);

            // do the range common band filter
            rangeCommonBandFilter(refSlc,
                                secSlc,
                                geometryIfgram,
                                geometryIfgramConj,
                                refSpectrum,
                                secSpectrum,
                                rangeFrequencies,
                                rangeFilter,
                                blockRows,
                                nfft);
                                
        }
        looksObj.ncols(nfft);
        // refAmplitudeLooked = sum(abs(refSlc)^2)
        looksObj.multilook(refSlc, refAmplitudeLooked, 2);
        looksObj.multilook(secSlc, secAmplitudeLooked, 2);

        // upsample the refernce and secondary SLCs
        refSignal.upsample(refSlc, refSlcUpsampled, blockRows, nfft, oversample, shiftImpact);
        secSignal.upsample(secSlc, secSlcUpsampled, blockRows, nfft, oversample, shiftImpact);
       
        // Compute oversampled interferogram data
        #pragma omp parallel for
        for (size_t line = 0; line < blockRowsData; line++){
            for (size_t col = 0; col < oversample*ncols; col++){
                ifgramUpsampled[line*(oversample*ncols) + col] = 
                        refSlcUpsampled[line*(oversample*nfft) + col]*
                        std::conj(secSlcUpsampled[line*(oversample*nfft) + col]);
            }
        }

        // Reclaim the extra oversample looks across
        float ov = oversample;
        #pragma omp parallel for
        for (size_t line = 0; line < blockRowsData; line++){
            for (size_t col = 0; col < ncols; col++){
                std::complex<float> sum =(0,0);
                for (size_t j=0; j< oversample; j++)
                    sum += ifgramUpsampled[line*(ncols*oversample) + j + col*oversample];
                ifgram[line*ncols + col] = sum/ov;            
            }
        }

        // Take looks down (summing columns)
        if (_doMultiLook){

            looksObj.ncols(ncols);
            looksObj.multilook(ifgram, ifgramMultiLooked);
            interferogram.setBlock(ifgramMultiLooked, 0, rowStart/_azimuthLooks,
                        ncols/_rangeLooks, blockRowsData/_azimuthLooks);

            if (_computeCoherence) {
                #pragma omp parallel for
                for (size_t i = 0; i< ifgramMultiLooked.size(); ++i){
                    coherence[i] = std::abs(ifgramMultiLooked[i])/
                            std::sqrt(refAmplitudeLooked[i]*secAmplitudeLooked[i]);
                }

                coherenceRaster.setBlock(coherence, 0, rowStart/_azimuthLooks,
                        ncols/_rangeLooks, blockRowsData/_azimuthLooks);
            }
        } else {
            // set the block of interferogram
            interferogram.setBlock(ifgram, 0, rowStart, ncols, blockRowsData);
        }
    }
}

/**
 * @param[in] oversample upsampling factor
 * @param[in] nfft fft length in range direction
 * @param[in] blockRows number of rows of the block of data
 * @param[out] shiftImpact frequency responce (a linear phase) to a sub-pixel shift in time domain introduced by upsampling followed by downsampling 
 */
void isce::signal::Crossmul::
lookdownShiftImpact(size_t oversample, size_t nfft, size_t blockRows, 
        std::valarray<std::complex<float>> &shiftImpact)
{
    // range frequencies given nfft and oversampling factor
    std::valarray<double> rangeFrequencies(oversample*nfft);

    // sampling interval in range
    double dt = 1.0/oversample;

    // get the vector of range frequencies
    //filter object
    isce::signal::Filter<float> tempFilter;
    tempFilter.fftfreq(dt, rangeFrequencies);

    // in the process of upsampling the SLCs, creating upsampled interferogram
    // and then looking down the upsampled interferogram to the original size of
    // the SLCs, a shift is introduced in range direction.
    // As an example for a signal with length of 5 and :
    // original sample locations:   0       1       2       3        4
    // upsampled sample locations:  0   0.5 1  1.5  2  2.5  3   3.5  4   4.5
    // Looked dow sample locations:   0.25    1.25    2.25    3.25    4.25
    // Obviously the signal after looking down would be shifted by 0.25 pixel in
    // range comared to the original signal. Since a shift in time domain introduces
    // a liner phase in frequency domain, we compute the impact in frequency domain.

    // the constant shift based on the oversampling factor
    double shift = 0.0;
    shift = (1.0 - 1.0/oversample)/2.0;

    // compute the frequency response of the subpixel shift in range direction
    std::valarray<std::complex<float>> shiftImpactLine(oversample*nfft);
    for (size_t col=0; col<shiftImpactLine.size(); ++col){
        double phase = -1.0*shift*2.0*M_PI*rangeFrequencies[col];
        shiftImpactLine[col] = std::complex<float> (std::cos(phase),
                                                    std::sin(phase));
    }

    // The imapct is the same for each range line. Therefore copying the line for the block
    for (size_t line = 0; line < blockRows; ++line){
            shiftImpact[std::slice(line*nfft*oversample, nfft*oversample, 1)] = shiftImpactLine;
    }
}

/**
* @param[in] refSlc a block of the reference SLC to be filtered
* @param[in] secSlc a block of second SLC to be filtered
* @param[in] geometryIfgram a simulated interferogram that contains the geometrical phase due to baseline separation
* @param[in] geometryIfgramConj conjugate of geometryIfgram
* @param[in] refSpectrum spectrum of geometryIfgramConj in range direction
* @param[in] secSpectrum spectrum of geometryIfgram in range direction
* @param[in] rangeFrequencies frequencies in range direction
* @param[in] rngFilter a filter object
* @param[in] blockLength number of rows
* @param[in] ncols number of columns
*/
void isce::signal::Crossmul::
rangeCommonBandFilter(std::valarray<std::complex<float>> &refSlc,
                        std::valarray<std::complex<float>> &secSlc,
                        std::valarray<std::complex<float>> geometryIfgram,
                        std::valarray<std::complex<float>> geometryIfgramConj,
                        std::valarray<std::complex<float>> &refSpectrum,
                        std::valarray<std::complex<float>> &secSpectrum,
                        std::valarray<double> &rangeFrequencies,
                        isce::signal::Filter<float> &rngFilter,
                        size_t blockLength,
                        size_t ncols)
{
    // size of the arrays
    size_t vectorLength = refSlc.size();

    // Aligning the spectrum of the two SLCs
    // Shifting the range spectrum of each image according to the local 
    // (slope-dependent) wavenumber. This shift in frequency domain is 
    // achieved by removing/adding the geometrical (representing topography)
    // from/to refernce and secondary SLCs in time domain.
    refSlc *= geometryIfgramConj;
    secSlc *= geometryIfgram;

    // range frequency shift
    double frequencyShift = 0.0;

    // determine the frequency shift based on the power spectral density of 
    // the geometrical interferometric pphase using an emprirical approach
    rangeFrequencyShift(refSpectrum,
                        secSpectrum,
                        rangeFrequencies,
                        blockLength,
                        ncols,
                        frequencyShift);

    std::cout << "frequencyShift : "<< frequencyShift << std::endl;
    std::cout << "range bandwidth: " << _rangeBandwidth << std::endl;

    // Since the spectrum of the ref and sec SLCs are already aligned,
    // we design the low-pass filter as a band-apss at zero frequency with
    // bandwidth of (W - frequency shift)
    std::valarray<double> filterCenterFrequency{0.0};
    std::valarray<double> filterBandwidth{_rangeBandwidth - frequencyShift};
    std::string filterType = "cosine";
    
    // Contruct the low pass filter for this block. This filter is
    // common for both SLCs
    rngFilter.constructRangeBandpassFilter(_rangeSamplingFrequency,
                                    filterCenterFrequency,
                                    filterBandwidth,
                                    ncols,
                                    blockLength,
                                    filterType);

    // low pass filter the ref and sec slc    
    rngFilter.filter(refSlc, refSpectrum);
    rngFilter.filter(secSlc, secSpectrum);

    // add/remove half geometrical phase to/from reference and secondary SLCs
    #pragma omp parallel for
    for (size_t i = 0; i < vectorLength; ++i){

        // Half phase due to baseline separation obtained from range difference
        // from refernce and secondary antennas to the target (i.e., range offset derived from
        // geometrical coregistration)
        double halfPhase = std::arg(geometryIfgram[i])/2.0;
        refSlc[i] *= std::complex<float> (std::cos(halfPhase), std::sin(halfPhase));
        secSlc[i] *= std::complex<float> (std::cos(halfPhase), -1*std::sin(halfPhase));

    }
}

