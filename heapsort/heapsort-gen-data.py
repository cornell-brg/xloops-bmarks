#=========================================================================
# heapsort-gen-data.py
#=========================================================================
# Script to generate data for heapsort. Usage:
# % python heapsort-gen-data.py > heapsort.dat

import random

dataset_sizes = [1024, 2048]
max_value = 100000

# generate random data based on the max/min values
def gen_rand( num_elements, max_value, min_value=1 ):
  return map( lambda x: random.choice( xrange( min_value, max_value ) ), \
              xrange( num_elements ) )

def print_var( name, value ):
  print "int {} = {};".format( name, value )


def print_arr( name, arr, type="int" ):
  str_temp = "{1} {0}[] = {{\n{2}}};\n"
  entry_temp = "  {0},\n"

  # put the array entries to the entry
  entries = ""
  for el in arr:
    entries += entry_temp.format( el )

  str_arr = str_temp.format( name, type, entries )
  print str_arr


random.seed( 0xdeadbeef )

# generate random input for the largest dataset
rand_arr = gen_rand( max(dataset_sizes), max_value, 0 )

print_arr( "A", rand_arr )

# print solutions for the various sizes
for dataset_size in dataset_sizes:
  print_arr( "A_ref{}".format( dataset_size ), \
             sorted( rand_arr[:dataset_size] ) )


