//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: ml
// Original code: Joshua Cohen
// Copyright 2018
//

#ifndef __ISCE_CORE_RASTER_H__
#define __ISCE_CORE_RASTER_H__

#include <complex>
#include <cstdint>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <valarray>
#include "gdal_priv.h"
#include "gdal_vrt.h"
#include "Constants.h"

namespace isce {
  namespace core {
    
    class Raster {
      
    public:
      
      // Constructors and destructors
      // Open existing file: (filename, [GA_ReadOnly or GDAL_Update])
      // Note from gdal.h: typedef enum { GA_ReadOnly=0, GA_Update=1 } GDALAccess; 
      // Create new file: (filename, width, length, numBands[=1], datatype[=default], GDALdriver[=default])
      //                  (filename, STL container to infer width and datatype, length)
      Raster(const std::string&);
      Raster(const std::string&, GDALAccess);
      Raster(const std::string&, size_t, size_t, size_t, GDALDataType, const std::string&);
      Raster(const std::string&, size_t, size_t, size_t, GDALDataType);
      Raster(const std::string&, size_t, size_t, size_t);
      Raster(const std::string&, size_t, size_t, GDALDataType);
      Raster(const std::string&, size_t, size_t);
      Raster(const std::string&, const Raster&);
      template<typename T> Raster(const std::string&, const std::vector<T>&,      size_t);
      template<typename T> Raster(const std::string&, const std::valarray<T>&,    size_t);
      Raster(const std::string&, const std::vector<Raster>&);
      Raster(const Raster&);
      ~Raster();

      // Operators, getters and setters
      inline Raster& operator=(const Raster&);
      inline GDALDataset* dataset()  const { return _dataset; }
      inline void         dataset(GDALDataset* ds) { _dataset=ds; }
      inline size_t       length()   const { return _dataset->GetRasterYSize(); }
      inline size_t       width()    const { return _dataset->GetRasterXSize(); }
      inline size_t       numBands() const { return _dataset->GetRasterCount(); }
      inline GDALAccess   access()   const { return _dataset->GetAccess(); }
      inline GDALDataType dtype(const size_t band=1) const { return _dataset->GetRasterBand(band)->GetRasterDataType(); }
      inline bool         match(const Raster & rast) const { return width()==rast.width() && length()==rast.length(); }  

      void open(const std::string&, GDALAccess);
      //void close() { GDALClose( _dataset ); }  // todo: fix segfault conflict with destructor
      
        
      // Pixel read/write, optional band index	  
      // (buffer, x-index, y-index, [band-index=1])
      template<typename T> void getSetValue(T&, size_t, size_t, size_t, GDALRWFlag);

      template<typename T> void getValue(T&, size_t, size_t, size_t);
      template<typename T> void getValue(T&, size_t, size_t);

      template<typename T> void setValue(T&, size_t, size_t, size_t);
      template<typename T> void setValue(T&, size_t, size_t);

     
      // Line read/write, generic container w/ width or STL container, optional band index
      // (buffer, line-index, nelem, [band-index=1])
      template<typename T> void getSetLine(T*, size_t, size_t, size_t, GDALRWFlag);
      
      template<typename T> void getLine(T*, size_t, size_t, size_t);
      template<typename T> void getLine(T*, size_t, size_t);
      template<typename T> void getLine(std::vector<T>&, size_t, size_t);
      template<typename T> void getLine(std::vector<T>&, size_t);
      template<typename T> void getLine(std::valarray<T>&,size_t,size_t);
      template<typename T> void getLine(std::valarray<T>&,size_t);
      
      template<typename T> void setLine(T*,size_t,size_t,size_t);
      template<typename T> void setLine(T*,size_t,size_t);
      template<typename T> void setLine(std::vector<T>&,size_t,size_t);
      template<typename T> void setLine(std::vector<T>&,size_t);
      template<typename T> void setLine(std::valarray<T>&,size_t,size_t);
      template<typename T> void setLine(std::valarray<T>&,size_t);

      
      // 2D block read/write, generic container w/ width or STL container, optional band index
      // (buffer, x-index, y-index, nXelem, nYelem, [band-index=1])
      template<typename T> void getSetBlock(T*,size_t,size_t,size_t,size_t,size_t, GDALRWFlag);

      template<typename T> void getBlock(T*,size_t,size_t,size_t,size_t,size_t);
      template<typename T> void getBlock(T*,size_t,size_t,size_t,size_t);
      template<typename T> void getBlock(std::vector<T>&,size_t,size_t,size_t,size_t,size_t);
      template<typename T> void getBlock(std::vector<T>&,size_t,size_t,size_t,size_t);
      template<typename T> void getBlock(std::valarray<T>&,size_t,size_t,size_t,size_t,size_t);
      template<typename T> void getBlock(std::valarray<T>&,size_t,size_t,size_t,size_t);

      template<typename T> void setBlock(T*,size_t,size_t,size_t,size_t,size_t);
      template<typename T> void setBlock(T*,size_t,size_t,size_t,size_t);
      template<typename T> void setBlock(std::vector<T>&,size_t,size_t,size_t,size_t,size_t);
      template<typename T> void setBlock(std::vector<T>&,size_t,size_t,size_t,size_t);
      template<typename T> void setBlock(std::valarray<T>&,size_t,size_t,size_t,size_t,size_t);
      template<typename T> void setBlock(std::valarray<T>&,size_t,size_t,size_t,size_t);
      
    private:
      
      GDALDataset * _dataset; 
      static const std::unordered_map<std::type_index, GDALDataType> _gdts;
    };
  }
}

#define ISCE_CORE_RASTER_ICC
#include "Raster.icc"
#undef ISCE_CORE_RASTER_ICC

#endif