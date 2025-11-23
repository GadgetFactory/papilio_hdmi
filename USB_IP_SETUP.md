# Instructions for Adding Gowin USB SoftPHY IP Core

## Overview
The **USB SoftPHY** IP core implements USB 1.1 Full-Speed using regular GPIO pins (not the hard USB controller). This is perfect for boards with USB routed to external pins like yours (G11/H12).

## Step 1: Open Gowin IDE
1. Open the Gowin FPGA Designer
2. Open your project: papilio_arcade_board.gprj

## Step 2: Add USB SoftPHY IP Core
1. In the IDE, go to: **Tools → IP Core Generator**
2. In the IP Core catalog, navigate to: **Communication → USBSoftPHY**
3. Double-click to open the IP configuration dialog

## Step 3: Configure USB SoftPHY IP
Configure with these settings:
- **Component Name**: `usb_softphy_core`
- **Device Mode**: Device (not Host)
- **USB Speed**: Full-Speed (12 Mbps)
- **Clock Frequency**: 48 MHz (you'll need to add a PLL for this)
- **Enable CDC-ACM**: Yes (for serial communication)
- **Endpoints**: Configure at least 2 (Control EP0 + Serial EP)

## Step 4: Generate
1. Click **OK** to generate
2. The IP will be generated in: `src/gowin_usb_softphy/`
3. Files created:
   - `usb_softphy_core.v` - Top-level wrapper
   - `usb_softphy_core.vo` - Encrypted implementation (if encrypted)
   - `usb_softphy_core_tmp.v` - Template for instantiation

## Step 5: Expected Interface
The USB SoftPHY IP should have ports like:
```verilog
module usb_softphy_core (
    input  clk_48mhz,     // 48MHz USB clock
    input  rst_n,         // Reset
    
    // USB PHY interface (connects to external pins)
    inout  usb_dp,        // D+ pin (G11)
    inout  usb_dn,        // D- pin (H12)
    
    // Endpoint interface
    output [7:0] rx_data,
    output       rx_valid,
    input        rx_ready,
    
    input  [7:0] tx_data,
    input        tx_valid,
    output       tx_ready,
    
    // Status
    output usb_enum_done, // Enumeration complete
    output usb_connected
);
```

## Step 6: Clock Generation
You'll need a 48MHz clock. We already have the PLL:
- Use existing `usb_pll.v` (27MHz → 48MHz)
- Or regenerate with proper settings

## Notes
- ✅ **Works with external GPIO pins** (unlike USB_DEVICE_CONTROLLER)
- ✅ **Full USB 1.1 protocol** in soft logic
- ✅ **CDC-ACM support** for serial communication
- ✅ **Compatible with your board's USB routing**

After generating, update `usb_serial_gowin.v` to instantiate the actual IP!
