//========================================================================
// Basic test of KMeans Scalar Implementation
//========================================================================

#include "kmeans-scalar.h"
#include "utst.h"

// Load dataset for testing
#include "kmeans-color100.dat"

//------------------------------------------------------------------------
// TestStage1
//------------------------------------------------------------------------

UTST_AUTO_TEST_CASE( TestStage1 )
{
  // Define some constants
  int ex_len = 18;
  int c_len = 2;
  int f_len = 2;

  // Create some input data
  float set0[ex_len * f_len];
  for ( int i = 0; i < ex_len; i++ ) {
    float temp = static_cast<float>(i);
    for ( int j = 0; j < f_len; j++ ) {
      set0[i*f_len + 0] = temp;
      set0[i*f_len + 1] = temp+0.2;
    }
  }
  int ex_2_cl[ex_len];
  float c[] = { 0, 0, 20, 20 };
  int ex_2_cl_ref[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 1, 1, 1, 1, 1, 1, 1, 1};

  // Test and verify
  kmeans::stage1_scalar( c, set0, ex_2_cl, ex_len, c_len, f_len );

  for ( int i = 0; i < ex_len; i++ )
    UTST_CHECK_EQ( ex_2_cl[i], ex_2_cl_ref[i] );
}

//------------------------------------------------------------------------
// TestStage2
//------------------------------------------------------------------------

UTST_AUTO_TEST_CASE( TestStage2 )
{
  // Define some constants
  int ex_len = 18;
  int c_len = 2;
  int f_len = 2;

  // Create some input data
  float set0[ex_len * f_len];
  for ( int i = 0; i < ex_len; i++ ) {
    float temp = static_cast<float>(i);
    for ( int j = 0; j < f_len; j++ ) {
      set0[i*f_len + 0] = temp;
      set0[i*f_len + 1] = temp+0.2;
    }
  }
  int ex_2_cl[ex_len];
  float c[] = { 0, 0, 20, 20 };
  float newc[] = { -1, -1, -1, -1 };
  float c_ref[] = { 4.5, 4.7, 13.5, 13.7 };

  // Test and verify
  kmeans::stage1_scalar( c, set0, ex_2_cl, ex_len, c_len, f_len );
  kmeans::stage2_scalar( newc, set0, ex_2_cl, ex_len, c_len, f_len );

  for ( int i = 0; i < c_len*f_len; i++ )
    UTST_CHECK_EQ( newc[i], c_ref[i] );
}

//------------------------------------------------------------------------
// TestSinglePass
//------------------------------------------------------------------------

UTST_AUTO_TEST_CASE( TestSinglePass )
{
  float delta, delta_ref = 1.0;

  float c[c_len * f_len];
  float c_ref[] = {
    0.194967, 1.708859, 0.435656, 2.666305, -0.224308,
    -3.313472, -0.000660, -0.469089, -0.636691,
    2.520223, -0.319096, -1.188386, -0.332860, -1.658888,
    -0.881189, 0.083349, -0.647248, -0.436155,
    2.594721, 0.794180, -2.238830, 2.733452, 0.465082,
    -3.915413, -1.134991, -1.068460, 0.109748,
    1.298655, 1.009534, -1.156180, 0.074074, -0.027620,
    -0.185348, -0.530554, -0.170689, 0.203193
  };
  int ex_2_cl[ex_len];

  kmeans::stage0_scalar( c, ex, ex_2_cl, ex_len, c_len, f_len );
  delta = kmeans::stage1_scalar( c, ex, ex_2_cl, ex_len, c_len, f_len );
  kmeans::stage2_scalar( c, ex, ex_2_cl, ex_len, c_len, f_len );

  UTST_CHECK_EQ( delta, delta_ref );
  for ( int i = 0; i < c_len*f_len; i++ )
    UTST_CHECK_EQ( c[i], c_ref[i] );
}

//------------------------------------------------------------------------
// TestConvergence1
//------------------------------------------------------------------------

UTST_AUTO_TEST_CASE( TestConvergence1 )
{
  UTST_CHECK_FAILED( "Disabling due to LLVM issue - shreesha" );
  float threshold = 0.1;

  float c[c_len * f_len];
  float c_ref[] = {
    0.089114, 1.674945, 0.622621, 2.560058, -0.144036,
    -3.046195, 0.045659, -0.445779, -0.738152,
    2.615783, -0.067940, -1.609957, -0.354439, -1.476825,
    -0.735965, -0.021397, -0.656011, -0.319356,
    2.533798, 0.873084, -2.234116, 2.734418, 0.483124,
    -3.928847, -1.089093, -0.991660, 0.115436,
    1.185163, 1.031028, -1.078820, 0.067238, 0.026578,
    -0.150551, -0.602972, -0.123678, 0.309425
  };

  kmeans::kmeans_scalar( c, ex, ex_len, c_len, f_len, threshold );

  for ( int i = 0; i < c_len*f_len; i++ )
    UTST_CHECK_EQ( c[i], c_ref[i] );
}

//------------------------------------------------------------------------
// TestConvergence2
//------------------------------------------------------------------------

UTST_AUTO_TEST_CASE( TestConvergence2 )
{
  UTST_CHECK_FAILED( "Disabling due to LLVM issue - shreesha" );
  float threshold = 0.001;

  float c[c_len * f_len];
  float c_ref[] = {
    -0.104355, 1.626173, 0.995669, 2.419428, 0.096445,
    -3.040048, -0.009745, -0.446268, -0.670082,
    2.290608, 0.603939, -1.900068, -0.031162, -0.951480,
    -0.571987, -0.183511, -0.362847, -0.121870,
    2.370237, 1.044574, -2.197306, 2.855925, 0.136545,
    -3.802903, -0.860706, -0.909265, -0.084076,
    0.479203, 0.809551, -0.073306, -0.136223, 0.436175,
    0.082388, -0.835809, -0.172531, 0.520966
  };

  kmeans::kmeans_scalar( c, ex, ex_len, c_len, f_len, threshold );

  for ( int i = 0; i < c_len*f_len; i++ )
    UTST_CHECK_EQ( c[i], c_ref[i] );
}

//------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
  utst::auto_command_line_driver( argc, argv );
}

