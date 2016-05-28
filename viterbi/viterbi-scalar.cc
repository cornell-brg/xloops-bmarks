//========================================================================
// viterbi-scalar.cc
//========================================================================
// Viterbi decoder based on implementation from CMU Spiral (spiral.net).
// Coefficients are stored in viterbi-details.h.

#include "viterbi-scalar.h"
#include "viterbi-details.h"



namespace viterbi {
  using namespace details;

  // Quickly generate a parity bit
  // see http://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
  //__attribute__ ((always_inline))
  int generate_symbol_bit_scalar(int x) {
    x ^= (x >> 16);
    x ^= (x >> 8);
    x ^= (x >> 4);
    x &= 0xf;
    return (0x6996 >> x) & 1;
  }

  // Viterbi Decoder: take encoded symbols and return the decoded msg
  void viterbi_scalar(unsigned char symbols[], unsigned char msg[],
                      int poly[], int framebits) {

    // Branch Table stores the state transitions, we only need to build
    // half thanks to trellace symmetry
    unsigned int branch_table[STATES/2 * rate];

    // shreesha:
    // IMPROTANT NOTE: Porting over XLOOPS changes to scalar code to make a
    // fair baseline comparison. It should not affect other projects and
    // will most likely result in better performance even when compiled
    // with the gcc based cross-compiler.

    // shreesha: XLOOPS - LLVM compiler saves branch_table base address
    // as a live-out in the first parallel loop which is referenced again
    // in the second parallel loop. Since, the array ends up being on the
    // stack the base address is computed and saved in the loop as a
    // live-out and hence violates the semantics of xloops. As a
    // workaround the address computation is moved outside of the loop by
    // adding the code below.

    unsigned int* branch_table_ptr = &branch_table[0];
    #ifdef _MIPS_ARCH_MAVEN
    __asm__ __volatile__
    (
      "addiu %[src0], %[src0], 0\n"
      : [src0] "+r" (branch_table_ptr)
    );
    #endif

    //--------------------------------------------------------------------
    // Build Branch Lookup Table
    //--------------------------------------------------------------------

    for (int state = 0; state < STATES/2; state++) {
      for (int i = 0; i < rate; i ++) {
        int bit = generate_symbol_bit_scalar(2*state & poly[i]);
        branch_table[i*STATES/2 + state] = bit ? 255 : 0;
      }
    }

    // Two buffers to store the accumulated error for each state/path
    unsigned int error1[STATES];
    unsigned int error2[STATES];
    // Pointers to track which accumulated error buffer is most current,
    // pointer targets are swapped after each frame bit
    unsigned int * old_error = error1;
    unsigned int * new_error = error2;
    // Record the minimum error path entering each state for all framebits
    int traces[framebits+(K-1)][STATES];

    // Bias the accumulated error buffer towards the known start state
    error1[0] = 0;
    for(int i=1;i<STATES;i++)
      error1[i] = 63;

    //--------------------------------------------------------------------
    // Calculate Forward Paths
    //--------------------------------------------------------------------
    // For each frame bit, accumulate errors and determine path entering
    // states i & i+(STATES/2) (evaluate simultaneously using symmetry)

    for (int s = 0; s < framebits + (K-1); s++) {
      for (int i = 0; i < STATES/2; i++) {

        int decision0, decision1;
        unsigned int metric, m0, m1, m2, m3;
        metric = 0;

        // Compute the error metric for this state
        for (int j = 0; j < rate; j++)
          metric += branch_table_ptr[i+j*STATES/2] ^ symbols[s*rate+j];

        const unsigned int max = rate*(256 - 1);

        m0 = old_error[i] + metric;
        m1 = old_error[i+STATES/2] + (max - metric);
        m2 = old_error[i] + (max - metric);
        m3 = old_error[i+STATES/2] + metric;

        // Determine which transition is minimum
        decision0 = m0 > m1;
        decision1 = m2 > m3;

        // Save error for minimum transition
        new_error[2*i] =   decision0 ? m1 : m0;
        new_error[2*i+1] = decision1 ? m3 : m2;

        // Save transmission bit for minimum transition
        traces[s][2*i]   = decision0;
        traces[s][2*i+1] = decision1;
      }

      //renormalize(new_error); // TODO: Necessary?

      // Swap targets of old and new error buffers
      unsigned int * temp = old_error;
      old_error = new_error;
      new_error = temp;
    }

    //--------------------------------------------------------------------
    // Path Traceback
    //--------------------------------------------------------------------
    // Traceback through the path with the lowest error and generate
    // the decoded message based on the observed state transitions

    // Assume final state is 0
    unsigned int endstate = 0;
    unsigned int nbits = framebits;

    // Offset to only access the last framebits bits (ignore flush bits)
    int (* trace_ptr)[STATES] = traces+(K-1);

    // Traceback loop, densely pack the message bits
    while (nbits-- !=0) {
      int k = trace_ptr[nbits][endstate];
      endstate = (endstate >> 1) | (k << (K-2));
      msg[nbits>>3] >>= 1;
      msg[nbits>>3] |= (k << 7);
    }
  }

}
