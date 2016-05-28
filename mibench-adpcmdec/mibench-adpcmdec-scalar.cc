//========================================================================
// mibench-adpcmdec-scalar.cc
//========================================================================

#include "mibench-adpcmdec-scalar.h"
#include <iostream>

namespace mibench_adpcmdec {

  void mibench_adpcmdec_scalar ( char indata[], short outdata[],
                                 int len, adpcm_state* state)
  {

    int delta       = 0;                    // Current adpcm output value
    int sign        = 0;                    // Current adpcm sign bit
    int inputbuffer = 0;                    // Place to keep the next 4-bit value
    int vpdiff      = 0;                    // Current change to valpred
    int valpred     = state->valprev;       // Predicted value
    int index       = state->index;         // Current step change index
    int step        = stepsizeTable[index]; // Stepsize
    int bufferstep  = 0;                    // Toggle between inputbuffer/input

    // Decode input
    for ( int i = 0; i < len ; i++ )
    {

      //------------------------------------------------------------------
	    // Step 1 - get the delta value
      //------------------------------------------------------------------

	    if ( bufferstep ) {
	      delta = inputbuffer & 0xf;
      }
	    else {
	        inputbuffer = indata[i/2];
	        delta       = (inputbuffer >> 4) & 0xf;
	    }
	    bufferstep = !bufferstep;

      //------------------------------------------------------------------
	    // Step 2 - Find new index value (for later)
      //------------------------------------------------------------------

	    index += indexTable[delta];
	    if ( index < INDEX_MIN ) index = INDEX_MIN;
	    if ( index > INDEX_MAX ) index = INDEX_MAX;

      //------------------------------------------------------------------
	    // Step 3 - Separate sign and magnitude
      //------------------------------------------------------------------

	    sign  = delta & 8;
	    delta = delta & 7;

      //------------------------------------------------------------------
	    // Step 4 - Compute difference and new predicted value
	    // Computes 'vpdiff = (delta+0.5)*step/4', but see comment
	    // in adpcm_coder.
      //------------------------------------------------------------------

	    vpdiff = step >> 3;
	    if ( delta & 4 ) vpdiff += step;
	    if ( delta & 2 ) vpdiff += step>>1;
	    if ( delta & 1 ) vpdiff += step>>2;

	    if ( sign )
	      valpred -= vpdiff;
	    else
	      valpred += vpdiff;

      //------------------------------------------------------------------
	    // Step 5 - clamp output value
      //------------------------------------------------------------------

	    if ( valpred > VALPRED_MAX )
	      valpred = VALPRED_MAX;
	    else if ( valpred < VALPRED_MIN )
	      valpred = VALPRED_MIN;

      //------------------------------------------------------------------
	    // Step 6 - Update step value
      //------------------------------------------------------------------

	    step = stepsizeTable[index];

      //------------------------------------------------------------------
	    // Step 7 - Output value
      //------------------------------------------------------------------

	    outdata[i] = valpred;

    }

  }

}
