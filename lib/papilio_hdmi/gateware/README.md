# Gateware for papilio_hdmi

This folder contains the HDL gateware sources used by the `papilio_hdmi` library. It includes the project's own modules (test patterns, TMDS encoder, timing, Wishbone wrappers, etc.).

Notes:
- Vendor-protected IP (for example the DVI transmitter IP in `src/dvi_tx/` which may be distributed under a separate vendor license or encrypted with `pragma protect`) is not copied into this folder automatically. If you need to include that IP in the library build, copy the files from the project's `src/dvi_tx/` and `src/gowin_rpll/` directories into `gateware/src/dvi_tx/` and `gateware/src/gowin_rpll/` respectively, preserving any license notices.
- This directory is intended to be a convenient package of gateware sources for users who want the full FPGA sources bundled with the PlatformIO library.

Usage:
- Use the files in `gateware/src/` when building or packaging the FPGA bitstream. The original repository `src/` contains the canonical sources and any encrypted/vendor IP.
