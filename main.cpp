#include "app.h"

// Read by the Synthux libDaisy fork's USB descriptors
const char* USBD_MANUFACTURER_STRING = "Synthux";
const char* USBD_PRODUCT_STRING_HS   = "Clessidra";
const char* USBD_PRODUCT_STRING_FS   = "Clessidra";

static clessidra::Application app;

extern "C" int main(void) {
    app.init();
    app.loop();
}
