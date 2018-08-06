//-*- C++ -*-
//-*- coding: utf-8 -*-
//
// Author: Bryan V. Riel, Joshua Cohen
// Copyright 2017-2018

#ifndef ISCE_CORE_BASIS_H
#define ISCE_CORE_BASIS_H

// isce::core
#include "Constants.h"
#include "LinAlg.h"

// Declaration
namespace isce {
    namespace core {
        class Basis;
    }
}

/** Simple class to store three-dimensional basis vectors*/
class isce::core::Basis {

    public:
        /** Default constructor*/
        Basis() {};

        /**Constructor with basis vectors*/
        Basis(cartesian_t & x0, cartesian_t & x1, cartesian_t & x2) :
            _x0(x0), _x1(x1), _x2(x2) {}
        
        /**Return first basis vector*/
        cartesian_t x0() const { return _x0; }

        /**Return second basis vector*/
        cartesian_t x1() const { return _x1; }

        /**Return third basis vector*/
        cartesian_t x2() const { return _x2; }
        
        /**Set the first basis vector*/
        void x0(cartesian_t & x0) { _x0 = x0; }

        /**Set the second basis vector*/
        void x1(cartesian_t & x1) { _x1 = x1; }

        /**Set the third basis vecot*/
        void x2(cartesian_t & x2) { _x2 = x2; }

        /** \brief Project a given vector onto basis
         *
         * @param[in] vec 3D vector to project
         * @param[out] res 3D vector output 
         *
         * \f[
         *      res_i = (x_i \cdot vec)
         *  \f] */
        inline void project(cartesian_t &vec, cartesian_t &res)
        {
            res[0] = LinAlg::dot(_x0, vec);
            res[1] = LinAlg::dot(_x1, vec);
            res[2] = LinAlg::dot(_x2, vec);
        };

        /** \brief Combine the basis with given weights
         *
         * @param[in] vec 3D vector to use as weights
         * @param[out] res 3D vector output
         *
         * \f[ 
         *      res = \sum_{i=0}^2 vec[i] \cdot x_i
         *  \f]*/
        inline void combine(cartesian_t &vec, cartesian_t &res)
        {
            for(int ii =0; ii < 3; ii++)
            {
                res[ii] = vec[0] * _x0[ii] + vec[1] * _x1[ii] + vec[2] * _x2[ii];
            }
        };

    private:
        cartesian_t _x0;
        cartesian_t _x1;
        cartesian_t _x2;
};
    
#endif

// end of file
