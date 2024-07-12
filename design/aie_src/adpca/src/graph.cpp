/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <adf.h>
#include "kernels.h"
#include "kernels/include.h"
#include "graph.h"

using namespace adf;

adpcaGraph mygraph;

int main(void) {

  float normalizedDoppler = NORMALIZED_DOPPLER;
  float normalizedSpatial = NORMALIZED_SPATIAL;

  mygraph.init();

  mygraph.run(NITER);

#ifdef INLCUDE_WEIGHT_APP
  mygraph.update(mygraph.normDop, normalizedDoppler);
  mygraph.update(mygraph.normSpa, normalizedSpatial);
#endif

  mygraph.end();
  return 0;
}
