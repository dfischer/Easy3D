/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EASY3D_CORE_CONSTANT_H
#define EASY3D_CORE_CONSTANT_H

#include <cfloat>
#include <climits>

namespace easy3d {

    /// \brief The PI.
    const double pi = 3.14159265358979323846264338327950288;
    /// \brief The half PI (PI/2)
    const double half_pi = pi * 0.5;
    /// \brief The quarter PI (PI/4)
    const double quarter_pi = pi * 0.25;
    /// \brief The two PI (2 * PI).
    const double two_pi	 = pi * 2.0;

    /// \brief Conversion from radian to degree
    double inline rad2deg(double rad) { return rad * 180.0 / pi; }
    /// \brief Conversion from degree to radian
    double inline deg2rad(double deg) { return deg * pi / 180.0; }

    /// Function returning minimum representable value for a given type
    template <typename FT> inline FT min();
    /// Function returning maximum representable value for a given type
    template <typename FT> inline FT max();

    // Template specializations for float and double

    /// \brief Function returning \c min for int type numbers.
    template <> inline int   min<int>() { return INT_MIN; }
    /// \brief Function returning \c max for int type numbers.
    template <> inline int   max<int>() { return INT_MAX; }
    /// \brief Function returning \c min for float type numbers.
    template <> inline float   min<float>() { return FLT_MIN; }
    /// \brief Function returning \c max for float type numbers.
    template <> inline float   max<float>() { return FLT_MAX; }
    /// \brief Function returning \c min for double type numbers.
    template <> inline double  min<double>() { return DBL_MIN; }
    /// \brief Function returning \c max for double type numbers.
    template <> inline double  max<double>() { return DBL_MAX; }

    /// Function returning the epsilon value for a given type
    template <typename FT> inline FT epsilon();
    /// Function returning the squared epsilon value for a given type
    template <typename FT> inline FT epsilon_sqr();

    // Template specializations for float and double
    /// \brief Function returning \c epsilon for float type numbers.
    template <> inline float  epsilon<float>() { return 1.0e-6f; }
    /// \brief Function returning \c squared epsilon for float type numbers.
    template <> inline float  epsilon_sqr<float>() { return 1.0e-12f; }
    /// \brief Function returning \c epsilon for double type numbers.
    template <> inline double epsilon<double>() { return 1.0e-12; }
    /// \brief Function returning \c squared epsilon for double type numbers.
    template <> inline double epsilon_sqr<double>() { return 1.0e-24; }
}


#endif // EASY3D_CORE_CONSTANT_H
