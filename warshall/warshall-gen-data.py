#=====================================================================
# Data generation script for warshall
#=====================================================================
# Creates a random N*N weight matrix and the expected value
# generate dataset using:
# % python warshall-gen-data.py > warshall.dat

import random

N = 32

def gen_arr( arr1, arr2, n ):
  for i in xrange( n ):
    arr1.append( [] )
    arr2.append( [] )
    for j in xrange( n ):
      # we use an exponential distribution because using uniform
      # distribution causes the reference to be mostly the same as src
      weight = random.expovariate(0.5)
      arr1[i].append( weight )
      arr2[i].append( weight )


def print_arr( arr, arr_name, n ):
  str_temp = "float {0}[N*N] = {{\n{1}}};\n"
  entry_temp = "  {0},\n"

  # put the array entries to the entry
  entries = ""
  for i in xrange( n ):
    for j in xrange( n ):
      entries += entry_temp.format( arr[i][j] )

  str_arr = str_temp.format( arr_name, entries )
  print str_arr

def print_dset( src_arr, ref_arr, n ):
  print "#define N {0}\n".format( n )

  print_arr( src_arr, "path_in",  N )
  print_arr( ref_arr, "path_ref", N )

# solve warshall algorithm
def solve_warshall( arr, n ):

  for k in xrange( n ):
    for i in xrange( n ):
      for j in xrange( n ):
        #print arr[i][j], (arr[i][k] + arr[k][j])
        if arr[i][j] > arr[i][k] + arr[k][j]:
          arr[i][j] = arr[i][k] + arr[k][j]


# set a fixed seed
random.seed( 0xdeadbeef )
src_arr = []
ref_arr = []
gen_arr( src_arr, ref_arr, N )
solve_warshall( ref_arr, N )

print_dset( src_arr, ref_arr, N )

