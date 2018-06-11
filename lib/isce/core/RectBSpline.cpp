//-*- C++ -*-
//-*- coding: utf-8 -*-

// std
#include <iostream>

#include "RectBSpline.h"
#include "FCMangle.h"

// Declare the Fortran interface for fitpack
// NOTE: Fortran requires pointers to arguments
extern "C" {

    // Compute spline approximation to grid data
    void regrid(int * iopt, int * mx, double * x, int * my, double * y, double * z,
                double * xb, double * xe, double * yb, double * ye, int * kx, int * ky,
                double * s, int * nxest, int * nyest, int * nx, double * tx,
                int * ny, double * ty, double * c, double * fp,
                double * wrk, int * lwrk, int * iwrk, int * kwrk, int * ierr);
   
    // Bivariate spline evaluation on a grid 
    void bispev(double * tx, int * nx, double * ty, int * ny, double * c, int * kx, int * ky,
                double * x, int * mx, double * y, int * my, double * z,
                double * wrk, int * lwrk, int * iwrk, int * kwrk, int * ierr);


}

// Constructor with spline initialization
isce::core::RectBSpline::
RectBSpline(std::valarray<double> & x, std::valarray<double> & y,
            std::valarray<double> & z, int kx, int ky, double s) {
    // Call initialization
    initialize(x, y, z, kx, ky, s);
}

// Initialize spline data
void isce::core::RectBSpline::
initialize(std::valarray<double> & x, std::valarray<double> & y,
           std::valarray<double> & z, int kx, int ky, double s) {

    // Set the spline degrees
    _kx = kx;
    _ky = ky;

    // Get sizes
    int mx = x.size();
    int my = y.size();

    // Default boundaries for spline approximation
    double xb = x[0];
    double xe = x[mx-1];
    double yb = y[0];
    double ye = y[my-1];

    // Check size of x-coordinates
    int nxest = mx + kx + 1;
    if (nxest < (2 * (kx + 1))) {
        std::cerr << "Not enough x coordinates for requested spline degree" << std::endl;
    }

    // Check size of y-coordinates
    int nyest = my + ky + 1;
    if (nyest < (2 * (ky + 1))) {
        std::cerr << "Not enough y coordinates for requested spline degree" << std::endl;
    }

    // Allocate knots
    _tx.resize(nxest, 0.0);
    _ty.resize(nyest, 0.0);
    _c.resize((nxest - kx - 1) * (nyest - ky - 1), 0.0);

    // Compute size of wrk array
    int u = std::max(my, nxest);
    int lwrk = 4 + nxest * (my + 2 * kx + 5) + nyest * (2 * ky + 5) +
               mx * (kx + 1) + my * (ky + 1) + u;

    // Allocate wrk
    std::valarray<double> wrk(0.0, lwrk);

    // Compute size of iwrk array
    int kwrk = 3 + mx + my + nxest + nyest;

    // Allocate iwrk
    std::valarray<int> iwrk(0, kwrk);

    // Input variables
    int iopt = 0;
    // Output variables
    int ierr;
    double fp = 0.0;
    
    // Call regrid from fitpack
    regrid(&iopt, &mx, &x[0], &my, &y[0], &z[0], &xb, &xe,
           &yb, &ye, &kx, &ky, &s, &nxest, &nyest, &_nx, &_tx[0], &_ny, &_ty[0], &_c[0],
           &fp, &wrk[0], &lwrk, &iwrk[0], &kwrk, &ierr);
    // Check status
    if (ierr > 0) {
        std::cerr << "Error in fitpack::regrid with code: " << ierr << std::endl;
        exit(0);
    }
}

// Evaluate at a scalar location
double isce::core::RectBSpline::
eval(double x, double y) {

    // Wrap scalars in 1-element valarrays
    std::valarray<double> xArr{x};
    std::valarray<double> yArr{y};
    std::valarray<double> zArr(1);

    // Call eval w/ valarrays
    eval(xArr, yArr, zArr);
    
    // Return only element of z
    return zArr[0]; 
}

// Evaluate at a grid of coordinates
void isce::core::RectBSpline::
eval(std::valarray<double> & x, std::valarray<double> & y, std::valarray<double> & z) {

    // Get sizes
    int mx = x.size();
    int my = y.size();

    // Allocate work arrays
    int lwrk = mx * (_kx + 1) + my * (_ky + 1);
    int kwrk = mx + my;
    std::valarray<double> wrk(lwrk);
    std::valarray<int> iwrk(kwrk);

    // Call bispev from fitpack
    int ierr;
    bispev(&_tx[0], &_nx, &_ty[0], &_ny, &_c[0], &_kx, &_ky, &x[0], &mx, &y[0], &my, &z[0],
           &wrk[0], &lwrk, &iwrk[0], &kwrk, &ierr);
    // Check status
    if (ierr > 0) {
        std::cerr << "Error in fitpack::bispev with code: " << ierr << std::endl;
        exit(0);
    }
}

// end of file
