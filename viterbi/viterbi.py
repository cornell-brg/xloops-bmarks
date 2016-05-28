#! /usr/bin/python2.6
# Convolutional Encoder and Hard-Decision Viterbi Decoder
# Code and example inspired by the tutorial at
# http://home.netcom.com/~chip.f/viterbi/algrthms.html


import itertools

K = 3
RATE = 1.0/2
POLY = [7, 5]

#***************************************************************
# CONVOLUTIONAL ENCODER FUNCTIONS
#***************************************************************

def encoder(input):
  """ Convolution encoder"""
  # NOTE: EXPECTS input FORMAT SUCH THAT LEFTMOST BIT IS MOST RECENT
  enc_out = []
  # push some extra 0s at into the input for flushing
  stream = [0]*(K-1) + input + [0]*(K-1)
  # shift in data right to left
  for i in reversed(range(len(input)+K-1)):
    symbol = generate_symbol(stream[i:i+K], POLY, K)
    # add to symbols
    enc_out.append(symbol)
  
  return enc_out

def generate_symbol(registers, polynomials, K): 
  """Convolutionally encode one bit of data, output the encoded symbol"""
  # NOTE: EXPECTS registers FORMAT SUCH THAT LEFTMOST BIT IS MOST RECENT
  xor = 0
  symbol = ''
  # for each symbol bit
  for p in polynomials:
    xor = 0
    # for each memory stage
    for k in range(K):
      # check polynomial to see if stage is connected
      # xor if there is a connection
      if (p >> k) & 1:
        xor ^= registers[k]
    # add digit to symbol
    symbol += str(xor)
  return symbol

def hamming(x, y):
  """Return the hamming distance between symbols x and y"""
  dist = 0
  for i in range(K-1): 
    if x[i] != y[i]:
      dist += 1
  return dist

def build_output_table():
  """Build an output table (find output given state and input bit)"""
  output = {}
  states = [''.join(x) for x in itertools.product('01', repeat=K-1)] 
  for s in states:
    output[s] = []
    stream = [int(x) for x in s]
    for input in [0, 1]:
      # NOTE: inputs arranged such that newest item is on left
      symbol = generate_symbol([input]+stream, POLY, K)
      output[s].append(symbol)
  return output 

def build_transition_table():
  """Build a transition table (find next state given state and input bit)"""
  transition = {}
  states = [''.join(x) for x in itertools.product('01', repeat=K-1)] 
  for s in states:
    transition[s] = []
    for input in [0, 1]:
      # NOTE: states are arranged such that newest item is left
      next_state = str(input) + s[:1]
      transition[s].append(next_state)
  return transition 

#***************************************************************
# VITERBI 
#***************************************************************

def viterbi(code):
  """Viterbi decoder"""

  # generate forward paths, errors
  errors, paths = forward_path(code)
  # then decode message during traceback
  message = traceback(code, errors, paths)
  return message


def forward_path(code):
  """Traverse the forward paths and accumulate errors"""

  # build the output table
  outputs = build_output_table()
  print "OUTPUT TABLE:"
  print outputs
  print

  # initialize data structures
  states = [''.join(x) for x in itertools.product('01', repeat=K-1)] 
  error = dict([(x, 0) for x in states])
  newerror = dict([(x, 0) for x in states])
  traces = dict([(x, []) for x in states])
  # TODO: initialization of error for states?
  # current implementation may not be accurate

  # for each input symbol received from the transmission stream
  for rx in code:
    # for all 2**(K-1) possible states
    for s in states:
      # generate the previous states connected to this state
      # (insert old bit on right, newer bit on left)
      ps0 = s[1:] + '0' # previous state 0
      ps1 = s[1:] + '1' # previous state 1
      input = int(s[0])
      # calculate the distance between the received symbol, i, and the
      # two potential symbols that could be emitted entering this state
      # (when previous state was 0x or 1x)
      E0 = hamming(rx, outputs[ps0][input])
      E1 = hamming(rx, outputs[ps1][input])
      # save the path with the smallest error
      if E0 <= E1:
        newerror[s] = error[ps0] + E0
        traces[s].append(ps0)
      else:
        newerror[s] = error[ps1] + E1
        traces[s].append(ps1)

    error = newerror.copy()
    print rx, error

  print "\nERRORS:", error

  return traces, error
        

def traceback(code, traces, error):
  """Traceback through path with smallest error and reconstruct message"""

  # build transition table (actually need a reverse lookup table for this)
  # TODO: generate reverse lookup table
  transition = build_transition_table()
  print "\nTRANSITION TABLE:"
  print transition
  print

  # initialize variables
  prev_state = ''
  best_error = 100  #TODO: init with max_value
  states = [''.join(x) for x in itertools.product('01', repeat=K-1)] 

  # find the path tail with the lowest error
  for s in states:
    if error[s] < best_error:
      best_error = error[s]
      prev_state = s

  # traceback to generate the path
  path = [prev_state]
  output = []
  for i in reversed(range(len(code))):
    prev_state = traces[prev_state][i]
    path.append(prev_state)

  path.reverse()

  # reconstruct the message
  message = []
  for i in range(len(path) - 1):
    s, ns = path[i:i+2]
    message.append(transition[s].index(ns))

  return message[:-(K-1)]


#***************************************************************
# MAIN
#***************************************************************
  
# CONVOLUTIONALLY ENCODE BITSTREAM
# in: mem: SYM p=7:   p=5: enc:
#  0   00    0^0^0  0^x^0   00 
#  1   00    1^0^0  1^x^0   11
#  0   10    0^1^0  0^x^0   10
#  1   01    1^0^1  1^x^1   00
#  1   10    1^1^0  1^x^0   01
#  1   11    1^1^1  1^x^1   10
#  0   11    0^1^1  0^x^1   01
#  0   01    0^0^1  0^x^1   11
#  1   00    1^0^0  1^x^0   11
#  0   10    0^1^0  0^x^0   10
#  1   01    1^0^1  1^x^1   00
#  0   10    0^1^0  0^x^0   10
#  0   01    0^0^1  0^x^1   11
#  0   00    0^0^0  0^x^0   00
#  1   00    1^0^0  1^x^0   11
# ---------FLUSH-------------
#  0   10    0^1^0  0^x^0   10
#  0   01    0^0^1  0^x^1   11
#      00    (final mem state)
# oldest data on left
data = [0, 1, 0, 1, 1, 
        1, 0, 0, 1, 0, 
        1, 0, 0, 0, 1]
# most recent data on left
bitstream = data[::-1]
symbols = encoder(bitstream)

print "VERIFY ENCODED BITSTREAM:"
# assert len(symbols) == len(sym_ref)
sym_ref = ['00', '11', '10', '00', '01', 
           '10', '01', '11', '11', '10', 
           '00', '10', '11', '00', '11', '10', '11']
for i in range(len(sym_ref)):
  print symbols[i], sym_ref[i], symbols[i] == sym_ref[i]
print


# DECODE SYMBOL STREAM USING VITERBI
# Viterbi Decoder Trellis Diagram (assuming 0 errors)
# TRELLIS
#  next_states:  00        01        10        11
#  prev_states: 00/01     10/11     00/01     10/11
#      outputs: 00/11     10/01     11/00     01/10
#
# time:  out:  branch errors (accumulated path errors):
#    0         *                                 
#    1    00   *0/2 ( 0)  1/1 ( 1)  2/0 ( 0)  1/1 ( 1)
#    2    11    2/0 ( 1)  1/1 ( 1) *0/2 ( 0)  1/1 ( 1)
#    3    10    1/1 (  ) *0/2 ( 0)  1/1 (  )  2/0 (  )
#    4    00    0/2 (  )  1/1 (  ) *2/0 ( 0)  1/1 (  )
#    5    01    1/1 (  )  0/2 (  )  1/1 (  ) *2/0 ( 0)
#    6    10    1/1 (  )  2/0 (  )  1/1 (  ) *0/2 ( 0)
#    7    01    1/1 (  ) *0/2 ( 0)  1/1 (  )  2/0 (  )
#    8    11   *2/0 ( 0)  1/1 (  )  0/2 (  )  1/1 (  )
#    9    11    2/0 (  )  1/1 (  ) *0/2 ( 0)  1/1 (  )
#   10    10    1/1 (  ) *2/0 ( 0)  1/1 (  )  0/2 (  )
#   11    00    0/2 (  )  1/1 (  ) *2/0 (  )  1/1 (  )
#   12    10    1/1 (  ) *2/0 ( 0)  1/1 (  )  0/2 (  )
#   13    11   *2/0 ( 0)  1/1 (  )  0/2 (  )  1/1 (  )
#   14    00   *0/2 ( 0)  1/1 (  )  2/0 (  )  1/1 (  )
#   15    11    2/0 (  )  1/1 (  ) *0/2 ( 0)  1/1 (  )
#   16    10    1/1 (  ) *2/0 ( 0)  1/1 (  )  0/2 (  )
#   17    11   *2/0 ( 0)  1/1 (  )  0/2 (  )  1/1 (  )

# There are two errors in this data, at t=3 and 
xmit = ['00', '11', '11', '00', '01', 
        '10', '01', '11', '11', '10', 
        '00', '00', '11', '00', '11', '10', '11']
message = viterbi(xmit)

print "VERIFY DECODED MESSAGE:"
# assert len(message) == len(data)
for i in range(len(data)):
  print message[i], data[i], message[i] == data[i]
