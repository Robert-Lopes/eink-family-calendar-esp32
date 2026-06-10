#include "display_driver.h"

GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 2> display(
    GxEPD2_750c_Z08(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY)
);
