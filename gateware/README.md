# Gateware for papilio_hdmi

This folder contains the HDL gateware sources used by the `papilio_hdmi` library. It includes the project's own modules (test patterns, TMDS encoder, timing, Wishbone wrappers, etc.).

## Modular Video Architecture

The HDMI gateware is designed with a **modular architecture** allowing users to include only the functionality they need:

### Core Components (Required)

| File | Description |
|------|-------------|
| `hdmi_phy_720p.v` | Shared HDMI physical layer - PLL, timing, TMDS, serializers |
| `tmds_encoder.v` | Open-source TMDS 8b/10b encoder with DC balance |
| `TMDS_rPLL.v` | PLL wrapper: 27MHz → 371.25MHz serial, 74.25MHz pixel |

### Video Mode Modules (Pick & Choose)

| File | Description | Resources |
|------|-------------|-----------|
| `wb_video_testpattern.v` | Test patterns (color bars, grid, grayscale) | Minimal |
| `wb_video_text.v` | 80x26 text mode, 16 colors, cursor, auto-advance | ~10KB BRAM |
| `wb_video_framebuffer.v` | 160x120 RGB332 → 720p with 6x scaling | ~20KB BRAM |

### Example Top-Level Integrations

| File | Description |
|------|-------------|
| `video_top_testpattern_only.v` | Minimal - test patterns only |
| `video_top_modular.v` | Full - all modes with runtime switching |

## Key Features

- **Open-Source TMDS**: No proprietary Gowin IP required
- **720p Output**: 1280x720 @ 60Hz
- **Wishbone Interface**: Register-based control from SPI/MCU
- **Gowin Primitives**: Uses rPLL, CLKDIV, OSER10, ELVDS_OBUF

## Clock Requirements

- Input: 27MHz reference clock
- Generated: 74.25MHz pixel clock, 371.25MHz serial clock

## Address Map (video_top_modular.v)

| Address Range | Module |
|---------------|--------|
| 0x0000-0x000F | Mode control |
| 0x0010-0x001F | Test pattern |
| 0x0020-0x00FF | Text mode |
| 0x0100-0x7FFF | Framebuffer |

## Usage Example

```verilog
// Minimal test pattern design:
hdmi_phy_720p u_phy (...);
wb_video_testpattern u_tp (...);

// Connect u_tp RGB outputs to u_phy RGB inputs
```

---

Notes:
- This directory is intended to be a convenient package of gateware sources for users who want the full FPGA sources bundled with the PlatformIO library.
- Use the files in `gateware/src/` when building or packaging the FPGA bitstream.
