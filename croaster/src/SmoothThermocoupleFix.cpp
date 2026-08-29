// Workaround for yuriisalimov/MAX6675_Thermocouple v2.0.2 link error.
//
// SmoothThermocouple.h declares:
//   static const int MIN_SMOOTHING_FACTOR = 2;
// but the library never provides an out-of-line definition. When compiled with
// optimization off (debug build, -O0/-Og) the use of MIN_SMOOTHING_FACTOR in
// SmoothThermocouple::setSmoothingFactor() (passed to max() by const-ref) is
// not folded to a constant, so linking fails with:
//   undefined reference to `SmoothThermocouple::MIN_SMOOTHING_FACTOR'
//
// Providing the missing definition here fixes the link for every build type
// without touching the dependency (which PlatformIO regenerates on update).

#include "SmoothThermocouple.h"

// No initializer here — the value is already given by the in-class
// initializer in SmoothThermocouple.h (`static const int ... = 2`).
const int SmoothThermocouple::MIN_SMOOTHING_FACTOR;
