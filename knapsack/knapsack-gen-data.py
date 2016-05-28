#=========================================================================
# knapsack-gen-data.py
#=========================================================================
# Script to generate data for knapsack. Usage:
# % python knapsack-gen-data.py > knapsack.dat

import random

max_weight   = 100
# minimum weight for "large" weight array
min_large_weight = 10
# maximum weight for "small" weight array
max_small_weight = 20
num_items    = 32
max_value    = 20


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

def solve_unbounded( weights, values, max_weight ):
  m = [ 0 for i in xrange( max_weight ) ]
  n = len( weights )
  for w in xrange( max_weight ):
    max_val = 0
    for i in xrange( n ):
      if weights[i] <= w:
        max_val = max( max_val, values[i] + m[ w - weights[i] ] )
    m[w] = max_val

  return m

random.seed( 0xdeadbeef )
density_arr      = gen_rand( num_items, max_value )
small_weight_arr = gen_rand( num_items, max_small_weight )
large_weight_arr = gen_rand( num_items, max_weight, min_large_weight )
small_value_arr  = map( lambda x: x[0]*x[1], \
                        zip( density_arr, small_weight_arr ) )
large_value_arr  = map( lambda x: x[0]*x[1], \
                        zip( density_arr, large_weight_arr ) )

print_var( "max_weight", max_weight )
print_var( "num_items",  num_items  )
print_arr( "small_value_arr",  small_value_arr  )
print_arr( "large_value_arr",  large_value_arr  )
print_arr( "small_weight_arr", small_weight_arr )
print_arr( "large_weight_arr", large_weight_arr )

small_ref = solve_unbounded( small_weight_arr, small_value_arr, max_weight )
large_ref = solve_unbounded( large_weight_arr, large_value_arr, max_weight )

print_arr( "small_ref", small_ref )
print_arr( "large_ref", large_ref )

