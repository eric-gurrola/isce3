//-*- C++ -*-
//-*- coding: utf-8 -*-

#ifndef RECTBSPLINE_H
#define RECTBSPLINE_H

#include <valarray>

// Declaration
namespace isce {
    namespace core {
        class RectBSpline;
    }
}

// Declaration of RectBSpline class
class isce::core::RectBSpline {

    public:
        // Default constructor
        RectBSpline() {};
        // Constructor with spline initialization
        RectBSpline(std::valarray<double> & x,
                    std::valarray<double> & y,
                    std::valarray<double> & z,
                    int kx=3,
                    int ky=3,
                    double s=0.0);

        // Initialize spline data
        void initialize(std::valarray<double> & x,
                        std::valarray<double> & y,
                        std::valarray<double> & z,
                        int kx=3,
                        int ky=3,
                        double s=0.0);

        // Evaluate spline for single coordinate
        double eval(double x, double y);

        // Evaluate spline for array of coordinates
        void eval(std::valarray<double> & x,
                  std::valarray<double> & y,
                  std::valarray<double> & z);

    private:
        // Spline degrees
        int _kx, _ky;

        // Knot x and y coordinates
        std::valarray<double> _tx;
        std::valarray<double> _ty;

        // Spline coefficients
        std::valarray<double> _c;

        // Working memory for spline evaluation
        int _nx, _ny, _lwrk;
};

#endif

// end of file
