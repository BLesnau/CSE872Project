#pragma once

#include "cptstd.hpp"
#include "matrix.hpp"

// Solve a linear system of equations of the form Ax = b
void solveEquations(cpt::Matrix<double,2>& A, cpt::Matrix<double,2> & b, cpt::Matrix<double,2>& x);

