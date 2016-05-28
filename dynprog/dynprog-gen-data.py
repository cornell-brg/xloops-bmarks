#=========================================================================
# dynprog-gen-data.py
#=========================================================================
# Generates a random W array. The app is not data dependent, so random is
# good enough. We don't do the output reference from here, need to run the
# program and output the results.
#
# Usage:
# % python dynprog-gen-data.py > dynprog.dat

import random

dim = 80
num_elements = dim * dim
max_value = 1000
min_value = 0

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
w_arr = gen_rand( num_elements, max_value, min_value )
print_var( "length", dim )
print_arr( "W", w_arr )

