// Copyright 2015 Hadrien Grasland
//
// This file is part of CLplusplus.
//
// CLplusplus is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CLplusplus is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CLplusplus.  If not, see <http://www.gnu.org/licenses/>.

// The code here can really apply to matrices of any kind without any modifications
typedef double MatrixElement;

// Here is a basic matrix multiplication algorithm, without any memory access optimization
//
// This function assumes that...
//    * The matrix has a size which is evenly divisible by the work group size
__kernel
void double_matmul_naive(__global const MatrixElement * restrict const in_A_matrix,
                         __global const MatrixElement * restrict const in_B_matrix,
                         __global       MatrixElement * restrict const out_matrix) {
   // Determine the current thread's location within the output matrix
   const unsigned int glob_output_i = get_global_id(0);
   const unsigned int glob_output_j = get_global_id(1);
   const unsigned int matrix_side_length = get_global_size(0);
   const size_t glob_output_idx = glob_output_j * matrix_side_length + glob_output_i;

   // Compute our part of the matrix multiplication
   const unsigned int A_input_j = glob_output_j;
   const unsigned int B_input_i = glob_output_i;
   MatrixElement result = 0.0;
   for(unsigned int glob_input_k = 0; glob_input_k < matrix_side_length; ++glob_input_k) {
      const size_t A_input_idx = A_input_j * matrix_side_length + glob_input_k;
      const size_t B_input_idx = glob_input_k * matrix_side_length + B_input_i;
      result += in_A_matrix[A_input_idx] * in_B_matrix[B_input_idx];
   }

   // Store the result
   out_matrix[glob_output_idx] = result;
}

// Here is an optimized algorithm which uses an optimized algorithm to improve memory coalescing
// It relies on the B matrix being initially transposed.
//
// This function assumes that...
//    * The matrix has a size which is evenly divisible by the work group size
__kernel
void double_matmul_transpose(__global const MatrixElement * restrict const in_A_matrix,
                             __global const MatrixElement * restrict const transposed_B_matrix,
                             __global       MatrixElement * restrict const out_matrix) {
   // Determine the current thread's location within the output matrix
   const unsigned int glob_output_i = get_global_id(0);
   const unsigned int glob_output_j = get_global_id(1);
   const unsigned int matrix_side_length = get_global_size(0);
   const size_t glob_output_idx = glob_output_j * matrix_side_length + glob_output_i;

   // Compute our part of the matrix multiplication
   const unsigned int glob_input_j = glob_output_j;
   MatrixElement result = 0.0;
   for(unsigned int glob_input_i = 0; glob_input_i < matrix_side_length; ++glob_input_i) {
      const size_t glob_input_idx = glob_input_j * matrix_side_length + glob_input_i;
      result += in_A_matrix[glob_input_idx] * transposed_B_matrix[glob_input_idx];
   }

   // Store the result
   out_matrix[glob_output_idx] = result;
}
