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

// Here is a basic matrix transpose algorithm, without any memory access optimization
//
// This function assumes that...
//    * The matrix has a size which is evenly divisible by the work group size
__kernel
void double_transpose_naive(__global const MatrixElement * restrict const in_matrix,
                            __global       MatrixElement * restrict const out_matrix) {
   // Determine the current thread's location within the input matrix
   const unsigned int glob_input_i = get_global_id(0);
   const unsigned int glob_input_j = get_global_id(1);
   const size_t glob_input_idx = glob_input_j * get_global_size(0) + glob_input_i;

   // Determine the corresponding output location in the transposed matrix
   const unsigned int glob_output_i = glob_input_j;
   const unsigned int glob_output_j = glob_input_i;
   const size_t glob_output_idx = glob_output_j * get_global_size(0) + glob_output_i;

   // Perform the matrix transposition
   out_matrix[glob_output_idx] = in_matrix[glob_input_idx];
}

// Here is a variant of the former algorithm where a local memory scratchpad is used to improve global memory access coalescing.
//
// This function assumes that...
//    * transpose_buf is allocated one double per work-item in a workgroup
//    * Work-groups are square
//    * The matrix has a size which is evenly divisible by the work group size
__kernel
void double_transpose_local(__global const MatrixElement * restrict const in_matrix,
                            __local        MatrixElement * restrict const transpose_buf,
                            __global       MatrixElement * restrict const out_matrix) {
   // Determine the current thread's location within the input matrix
   const unsigned int glob_input_i = get_global_id(0);
   const unsigned int glob_input_j = get_global_id(1);
   const size_t glob_input_idx = glob_input_j * get_global_size(0) + glob_input_i;

   // Determine a corresponding transposed location inside the local scratchpad
   const unsigned int loc_input_i = get_local_id(1);
   const unsigned int loc_input_j = get_local_id(0);
   const unsigned int loc_input_idx = loc_input_j * get_local_size(0) + loc_input_i;

   // Load our input matrix slide into the local scratchpad, in transposed form
   // (global reads are coalesced, local writes aren't but it doesn't matter as much)
   transpose_buf[loc_input_idx] = in_matrix[glob_input_idx];
   barrier(CLK_LOCAL_MEM_FENCE);

   // Since our local buffer is now transposed, we can access it normally for the output stage
   const unsigned int loc_output_i = get_local_id(0);
   const unsigned int loc_output_j = get_local_id(1);
   const unsigned int loc_output_idx = loc_output_j * get_local_size(0) + loc_output_i;   

   // We will want, however, to write the resulting block at a transposed location within the matrix
   const unsigned int output_offset_i = get_group_id(1) * get_local_size(0);
   const unsigned int output_offset_j = get_group_id(0) * get_local_size(1);

   // Determine the corresponding output location in the transposed matrix
   const unsigned int glob_output_i = output_offset_i + loc_output_i;
   const unsigned int glob_output_j = output_offset_j + loc_output_j;
   const size_t glob_output_idx = glob_output_j * get_global_size(0) + glob_output_i;

   // Commit the matrix transpose output to memory (global writes are now coalesced)
   out_matrix[glob_output_idx] = transpose_buf[loc_output_idx];
}
