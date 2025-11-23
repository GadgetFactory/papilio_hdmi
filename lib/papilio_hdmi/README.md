# papilio_hdmi

PlatformIO/Arduino library to control the Papilio Arcade Board FPGA HDMI output and RGB LED via a Wishbone-over-SPI interface.

Usage
- Include `#include <HDMIController.h>` in your sketch.
- Create `HDMIController hdmi(nullptr, csPin, clkPin, mosiPin, misoPin);` and call `hdmi.begin()` in `setup()`.

See the example in `examples/papilio_hdmi_example/`.
