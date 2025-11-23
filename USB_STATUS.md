# USB Implementation Status

## Problem Discovered

The Gowin `USBTOMULTISERIAL` IP core is designed for the **GW2A's internal hard USB controller**, not for external GPIO pins (usb_dp/usb_dn at G11/H12).

### Why It Doesn't Work:
- Gowin USB IP uses the FPGA's built-in USB 2.0 PHY hardware
- This hard block has dedicated pins (not user-configurable GPIO)
- Your board has USB signals routed to regular GPIO pins G11/H12
- The IP cannot drive external pins - it's hardwired to internal USB logic

## Options Forward

### Option 1: External USB-to-Serial Chip (RECOMMENDED)
**Use a dedicated USB bridge IC:**
- FT232RL (FTDI) - Most popular, good drivers
- CH340G/CH341 - Cheap, widely available  
- CP2102 (Silicon Labs) - Good performance

**Advantages:**
- ✅ Guaranteed to work
- ✅ No FPGA resources used
- ✅ OS drivers available
- ✅ Simple UART interface to FPGA

**Wiring:**
```
[PC USB] ↔ [FT232] ↔ [UART] ↔ [FPGA GPIO]
                      TX/RX
```

Connect FT232 TX/RX to spare FPGA pins, use existing UART module.

### Option 2: Soft USB PHY (Complex)
**Implement USB protocol in FPGA fabric:**

Attempted libraries:
1. ❌ **TinyFPGA USB** - Gowin synthesizer rejects constant concatenations
2. ❌ **no2usb** - Uses iCE40-specific primitives (SB_RAM40_4K, SB_IO)
3. ❌ **Gowin IP** - Only for internal hard USB, not GPIO pins

**To make soft PHY work, would need:**
- Rewrite TinyFPGA/no2usb for Gowin compatibility
- Replace all vendor primitives with generic RTL
- Handle 48MHz clock domain (current: 27MHz system)
- Implement USB 1.1 Full Speed protocol (enumeration, descriptors, CDC-ACM)
- ~5000+ lines of code, significant effort

### Option 3: Use Different FPGA Board
If you need USB built-in:
- TinyFPGA BX (has working USB bootloader)
- iCEBreaker (iCE40 with USB)
- Gowin development boards with hard USB routed correctly

##Current Implementation

The project now builds with a **stub USB module** that:
- ✅ Provides Wishbone register interface (0x20-0x22)
- ✅ Compiles cleanly  
- ✅ ESP32 can read/write registers
- ❌ No actual USB communication (just placeholder)

### Register Map (Non-functional USB):
- `0x20`: TX/RX data (reads return 0x00)
- `0x21`: Status (always shows: tx_ready=1, rx_valid=0, connected=0)
- `0x22`: Control

## Recommendation

**Add external FT232 module:**

1. **Get FT232 breakout board** (~$3-5 on AliExpress/Amazon)
2. **Connect to FPGA:**
   - FT232 TX → FPGA GPIO (e.g., pin A1)
   - FT232 RX → FPGA GPIO (e.g., pin B1)
   - GND → GND
3. **Use existing UART module** or create simple one
4. **Map to Wishbone** (same 0x20-0x22 registers)

This gives you working USB serial in ~1 hour instead of weeks of USB PHY development.

## Build Status

✅ **Project compiles successfully** with stub USB module
✅ HDMI video works
✅ RGB LED works  
✅ Wishbone bus functional
✅ ESP32 can access all peripherals (HDMI, LED, USB registers)

The system is **fully functional** except for actual USB communication.
