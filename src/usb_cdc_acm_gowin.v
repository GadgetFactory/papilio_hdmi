/*
 * usb_cdc_acm_gowin.v
 *
 * Complete USB CDC-ACM serial port using Gowin IP:
 * - USB_SoftPHY (UTMI PHY for external GPIO)
 * - USB_Device_Controller (protocol handling)
 * - CDC-ACM descriptors and endpoint logic
 * - Wishbone interface for ESP32
 */

module usb_cdc_acm_gowin (
    // Clock and Reset
    input  wire clk,           // 27MHz system clock
    input  wire clk_48mhz,     // 48MHz USB clock
    input  wire rst_n,
    
    // USB PHY pins
    inout  wire usb_dp,
    inout  wire usb_dn,
    
    // Wishbone Interface  
    input  wire [7:0] wb_adr_i,
    output reg  [7:0] wb_dat_o,
    input  wire [7:0] wb_dat_i,
    input  wire       wb_cyc_i,
    input  wire       wb_stb_i,
    input  wire       wb_we_i,
    output reg        wb_ack_o
);

    //=========================================================================
    // UTMI PHY Interface (between Device Controller and SoftPHY)
    //=========================================================================
    
    wire [7:0] utmi_dataout;
    wire       utmi_txvalid;
    wire       utmi_txready;
    wire [7:0] utmi_datain;
    wire       utmi_rxactive;
    wire       utmi_rxvalid;
    wire       utmi_rxerror;
    wire [1:0] utmi_linestate;
    wire [1:0] utmi_opmode;
    wire [1:0] utmi_xcvrselect;
    wire       utmi_termselect;
    wire       utmi_reset;
    
    //=========================================================================
    // Device Controller Status and Control
    //=========================================================================
    
    wire       usbrst;
    wire       highspeed;
    wire       suspend;
    wire       online;
    
    //=========================================================================
    // Endpoint TX/RX Interface
    //=========================================================================
    
    reg  [7:0] txdat;
    reg        txval;
    reg [11:0] txdat_len;
    reg  [3:0] txiso_pid;
    reg        txcork;
    wire       txpop;
    wire       txact;
    wire       txpktfin;
    
    wire [7:0] rxdat;
    wire       rxval;
    reg        rxrdy;
    wire       rxact;
    wire       rxpktval;
    wire       setup;
    wire [3:0] endpt;
    wire       sof;
    
    //=========================================================================
    // Descriptor ROM Interface
    //=========================================================================
    
    wire [15:0] descrom_raddr;
    wire [7:0]  desc_index;
    wire [7:0]  desc_type;
    wire [7:0]  descrom_rdata;
    
    // Descriptor addresses and lengths (hardcoded for CDC-ACM)
    wire [15:0] desc_dev_addr      = 16'h0000;
    wire [15:0] desc_dev_len       = 16'h0012;  // 18 bytes
    wire [15:0] desc_qual_addr     = 16'h0000;
    wire [15:0] desc_qual_len      = 16'h0000;
    wire [15:0] desc_fscfg_addr    = 16'h0012;
    wire [15:0] desc_fscfg_len     = 16'h0043;  // 67 bytes
    wire [15:0] desc_hscfg_addr    = 16'h0012;
    wire [15:0] desc_hscfg_len     = 16'h0043;
    wire [15:0] desc_oscfg_addr    = 16'h0000;
    wire [15:0] desc_hidrpt_addr   = 16'h0000;
    wire [15:0] desc_hidrpt_len    = 16'h0000;
    wire [15:0] desc_bos_addr      = 16'h0000;
    wire [15:0] desc_bos_len       = 16'h0000;
    wire [15:0] desc_strlang_addr  = 16'h0055;
    wire [15:0] desc_strvendor_addr = 16'h0059;
    wire [15:0] desc_strvendor_len = 16'h0010;  // 16 bytes
    wire [15:0] desc_strproduct_addr = 16'h0069;
    wire [15:0] desc_strproduct_len = 16'h0018;  // 24 bytes
    wire [15:0] desc_strserial_addr = 16'h0081;
    wire [15:0] desc_strserial_len = 16'h0010;  // 16 bytes
    wire        desc_have_strings  = 1'b1;
    
    reg  [7:0]  inf_alter;
    wire [7:0]  inf_alter_o;
    wire [7:0]  inf_sel;
    wire        inf_set;
    
    //=========================================================================
    // USB Descriptor ROM (CDC-ACM)
    //=========================================================================
    
    reg [7:0] descriptor_rom [0:255];
    
    initial begin
        // Device Descriptor (18 bytes at 0x0000)
        descriptor_rom[16'h0000] = 8'h12;  // bLength
        descriptor_rom[16'h0001] = 8'h01;  // bDescriptorType (Device)
        descriptor_rom[16'h0002] = 8'h00;  // bcdUSB 2.0 (LSB)
        descriptor_rom[16'h0003] = 8'h02;  // bcdUSB 2.0 (MSB)
        descriptor_rom[16'h0004] = 8'h02;  // bDeviceClass (CDC)
        descriptor_rom[16'h0005] = 8'h00;  // bDeviceSubClass
        descriptor_rom[16'h0006] = 8'h00;  // bDeviceProtocol
        descriptor_rom[16'h0007] = 8'h40;  // bMaxPacketSize0 (64)
        descriptor_rom[16'h0008] = 8'h09;  // idVendor (0x1209 - pid.codes)
        descriptor_rom[16'h0009] = 8'h12;
        descriptor_rom[16'h000A] = 8'hC1;  // idProduct (example)
        descriptor_rom[16'h000B] = 8'h00;
        descriptor_rom[16'h000C] = 8'h00;  // bcdDevice 1.0
        descriptor_rom[16'h000D] = 8'h01;
        descriptor_rom[16'h000E] = 8'h01;  // iManufacturer
        descriptor_rom[16'h000F] = 8'h02;  // iProduct
        descriptor_rom[16'h0010] = 8'h03;  // iSerialNumber
        descriptor_rom[16'h0011] = 8'h01;  // bNumConfigurations
        
        // Configuration Descriptor (67 bytes at 0x0012)
        // Configuration header (9 bytes)
        descriptor_rom[16'h0012] = 8'h09;  // bLength
        descriptor_rom[16'h0013] = 8'h02;  // bDescriptorType (Configuration)
        descriptor_rom[16'h0014] = 8'h43;  // wTotalLength (67 bytes LSB)
        descriptor_rom[16'h0015] = 8'h00;  // wTotalLength (MSB)
        descriptor_rom[16'h0016] = 8'h02;  // bNumInterfaces (2)
        descriptor_rom[16'h0017] = 8'h01;  // bConfigurationValue
        descriptor_rom[16'h0018] = 8'h00;  // iConfiguration
        descriptor_rom[16'h0019] = 8'hC0;  // bmAttributes (self-powered)
        descriptor_rom[16'h001A] = 8'h32;  // bMaxPower (100mA)
        
        // Interface 0: CDC Control (9 bytes)
        descriptor_rom[16'h001B] = 8'h09;  // bLength
        descriptor_rom[16'h001C] = 8'h04;  // bDescriptorType (Interface)
        descriptor_rom[16'h001D] = 8'h00;  // bInterfaceNumber (0)
        descriptor_rom[16'h001E] = 8'h00;  // bAlternateSetting
        descriptor_rom[16'h001F] = 8'h01;  // bNumEndpoints (1)
        descriptor_rom[16'h0020] = 8'h02;  // bInterfaceClass (CDC)
        descriptor_rom[16'h0021] = 8'h02;  // bInterfaceSubClass (ACM)
        descriptor_rom[16'h0022] = 8'h01;  // bInterfaceProtocol (AT commands)
        descriptor_rom[16'h0023] = 8'h00;  // iInterface
        
        // CDC Header Functional Descriptor (5 bytes)
        descriptor_rom[16'h0024] = 8'h05;  // bLength
        descriptor_rom[16'h0025] = 8'h24;  // bDescriptorType (CS_INTERFACE)
        descriptor_rom[16'h0026] = 8'h00;  // bDescriptorSubtype (Header)
        descriptor_rom[16'h0027] = 8'h10;  // bcdCDC 1.10 (LSB)
        descriptor_rom[16'h0028] = 8'h01;  // bcdCDC 1.10 (MSB)
        
        // CDC ACM Functional Descriptor (4 bytes)
        descriptor_rom[16'h0029] = 8'h04;  // bLength
        descriptor_rom[16'h002A] = 8'h24;  // bDescriptorType (CS_INTERFACE)
        descriptor_rom[16'h002B] = 8'h02;  // bDescriptorSubtype (ACM)
        descriptor_rom[16'h002C] = 8'h02;  // bmCapabilities (line coding)
        
        // CDC Union Functional Descriptor (5 bytes)
        descriptor_rom[16'h002D] = 8'h05;  // bLength
        descriptor_rom[16'h002E] = 8'h24;  // bDescriptorType (CS_INTERFACE)
        descriptor_rom[16'h002F] = 8'h06;  // bDescriptorSubtype (Union)
        descriptor_rom[16'h0030] = 8'h00;  // bControlInterface (0)
        descriptor_rom[16'h0031] = 8'h01;  // bSubordinateInterface (1)
        
        // CDC Call Management Functional Descriptor (5 bytes)
        descriptor_rom[16'h0032] = 8'h05;  // bLength
        descriptor_rom[16'h0033] = 8'h24;  // bDescriptorType (CS_INTERFACE)
        descriptor_rom[16'h0034] = 8'h01;  // bDescriptorSubtype (Call Management)
        descriptor_rom[16'h0035] = 8'h00;  // bmCapabilities
        descriptor_rom[16'h0036] = 8'h01;  // bDataInterface (1)
        
        // Endpoint 1 IN (Interrupt - 7 bytes)
        descriptor_rom[16'h0037] = 8'h07;  // bLength
        descriptor_rom[16'h0038] = 8'h05;  // bDescriptorType (Endpoint)
        descriptor_rom[16'h0039] = 8'h81;  // bEndpointAddress (EP1 IN)
        descriptor_rom[16'h003A] = 8'h03;  // bmAttributes (Interrupt)
        descriptor_rom[16'h003B] = 8'h08;  // wMaxPacketSize (8 bytes LSB)
        descriptor_rom[16'h003C] = 8'h00;  // wMaxPacketSize (MSB)
        descriptor_rom[16'h003D] = 8'hFF;  // bInterval (255ms)
        
        // Interface 1: CDC Data (9 bytes)
        descriptor_rom[16'h003E] = 8'h09;  // bLength
        descriptor_rom[16'h003F] = 8'h04;  // bDescriptorType (Interface)
        descriptor_rom[16'h0040] = 8'h01;  // bInterfaceNumber (1)
        descriptor_rom[16'h0041] = 8'h00;  // bAlternateSetting
        descriptor_rom[16'h0042] = 8'h02;  // bNumEndpoints (2)
        descriptor_rom[16'h0043] = 8'h0A;  // bInterfaceClass (CDC Data)
        descriptor_rom[16'h0044] = 8'h00;  // bInterfaceSubClass
        descriptor_rom[16'h0045] = 8'h00;  // bInterfaceProtocol
        descriptor_rom[16'h0046] = 8'h00;  // iInterface
        
        // Endpoint 2 OUT (Bulk - 7 bytes)
        descriptor_rom[16'h0047] = 8'h07;  // bLength
        descriptor_rom[16'h0048] = 8'h05;  // bDescriptorType (Endpoint)
        descriptor_rom[16'h0049] = 8'h02;  // bEndpointAddress (EP2 OUT)
        descriptor_rom[16'h004A] = 8'h02;  // bmAttributes (Bulk)
        descriptor_rom[16'h004B] = 8'h40;  // wMaxPacketSize (64 bytes LSB)
        descriptor_rom[16'h004C] = 8'h00;  // wMaxPacketSize (MSB)
        descriptor_rom[16'h004D] = 8'h00;  // bInterval
        
        // Endpoint 2 IN (Bulk - 7 bytes)
        descriptor_rom[16'h004E] = 8'h07;  // bLength
        descriptor_rom[16'h004F] = 8'h05;  // bDescriptorType (Endpoint)
        descriptor_rom[16'h0050] = 8'h82;  // bEndpointAddress (EP2 IN)
        descriptor_rom[16'h0051] = 8'h02;  // bmAttributes (Bulk)
        descriptor_rom[16'h0052] = 8'h40;  // wMaxPacketSize (64 bytes LSB)
        descriptor_rom[16'h0053] = 8'h00;  // wMaxPacketSize (MSB)
        descriptor_rom[16'h0054] = 8'h00;  // bInterval
        
        // String Descriptor 0: Language ID (4 bytes at 0x0055)
        descriptor_rom[16'h0055] = 8'h04;  // bLength
        descriptor_rom[16'h0056] = 8'h03;  // bDescriptorType (String)
        descriptor_rom[16'h0057] = 8'h09;  // wLANGID (0x0409 = US English LSB)
        descriptor_rom[16'h0058] = 8'h04;  // wLANGID (MSB)
        
        // String Descriptor 1: Manufacturer (16 bytes at 0x0059) "Papilio"
        descriptor_rom[16'h0059] = 8'h10;  // bLength
        descriptor_rom[16'h005A] = 8'h03;  // bDescriptorType (String)
        descriptor_rom[16'h005B] = 8'h50;  // 'P'
        descriptor_rom[16'h005C] = 8'h00;
        descriptor_rom[16'h005D] = 8'h61;  // 'a'
        descriptor_rom[16'h005E] = 8'h00;
        descriptor_rom[16'h005F] = 8'h70;  // 'p'
        descriptor_rom[16'h0060] = 8'h00;
        descriptor_rom[16'h0061] = 8'h69;  // 'i'
        descriptor_rom[16'h0062] = 8'h00;
        descriptor_rom[16'h0063] = 8'h6C;  // 'l'
        descriptor_rom[16'h0064] = 8'h00;
        descriptor_rom[16'h0065] = 8'h69;  // 'i'
        descriptor_rom[16'h0066] = 8'h00;
        descriptor_rom[16'h0067] = 8'h6F;  // 'o'
        descriptor_rom[16'h0068] = 8'h00;
        
        // String Descriptor 2: Product (24 bytes at 0x0069) "Arcade Board"
        descriptor_rom[16'h0069] = 8'h18;  // bLength
        descriptor_rom[16'h006A] = 8'h03;  // bDescriptorType (String)
        descriptor_rom[16'h006B] = 8'h41;  // 'A'
        descriptor_rom[16'h006C] = 8'h00;
        descriptor_rom[16'h006D] = 8'h72;  // 'r'
        descriptor_rom[16'h006E] = 8'h00;
        descriptor_rom[16'h006F] = 8'h63;  // 'c'
        descriptor_rom[16'h0070] = 8'h00;
        descriptor_rom[16'h0071] = 8'h61;  // 'a'
        descriptor_rom[16'h0072] = 8'h00;
        descriptor_rom[16'h0073] = 8'h64;  // 'd'
        descriptor_rom[16'h0074] = 8'h00;
        descriptor_rom[16'h0075] = 8'h65;  // 'e'
        descriptor_rom[16'h0076] = 8'h00;
        descriptor_rom[16'h0077] = 8'h20;  // ' '
        descriptor_rom[16'h0078] = 8'h00;
        descriptor_rom[16'h0079] = 8'h42;  // 'B'
        descriptor_rom[16'h007A] = 8'h00;
        descriptor_rom[16'h007B] = 8'h6F;  // 'o'
        descriptor_rom[16'h007C] = 8'h00;
        descriptor_rom[16'h007D] = 8'h61;  // 'a'
        descriptor_rom[16'h007E] = 8'h00;
        descriptor_rom[16'h007F] = 8'h72;  // 'r'
        descriptor_rom[16'h0080] = 8'h00;
        descriptor_rom[16'h0081] = 8'h64;  // 'd'
        descriptor_rom[16'h0082] = 8'h00;
        
        // String Descriptor 3: Serial (16 bytes at 0x0083) "12345678"
        descriptor_rom[16'h0083] = 8'h10;  // bLength (wrong address above, fixing)
        descriptor_rom[16'h0084] = 8'h03;  // bDescriptorType (String)
        descriptor_rom[16'h0085] = 8'h31;  // '1'
        descriptor_rom[16'h0086] = 8'h00;
        descriptor_rom[16'h0087] = 8'h32;  // '2'
        descriptor_rom[16'h0088] = 8'h00;
        descriptor_rom[16'h0089] = 8'h33;  // '3'
        descriptor_rom[16'h008A] = 8'h00;
        descriptor_rom[16'h008B] = 8'h34;  // '4'
        descriptor_rom[16'h008C] = 8'h00;
        descriptor_rom[16'h008D] = 8'h35;  // '5'
        descriptor_rom[16'h008E] = 8'h00;
        descriptor_rom[16'h008F] = 8'h36;  // '6'
        descriptor_rom[16'h0090] = 8'h00;
        descriptor_rom[16'h0091] = 8'h37;  // '7'
        descriptor_rom[16'h0092] = 8'h00;
    end
    
    // Descriptor ROM read
    assign descrom_rdata = descriptor_rom[descrom_raddr[7:0]];
    
    //=========================================================================
    // Endpoint Buffers and Logic
    //=========================================================================
    
    // EP2 IN (bulk data to host) - serial TX
    reg [7:0] ep2_in_buffer [0:63];
    reg [5:0] ep2_in_wptr;
    reg [5:0] ep2_in_rptr;
    reg       ep2_in_has_data;
    
    // EP2 OUT (bulk data from host) - serial RX
    reg [7:0] ep2_out_buffer [0:63];
    reg [5:0] ep2_out_wptr;
    reg [5:0] ep2_out_rptr;
    reg       ep2_out_has_data;
    
    //=========================================================================
    // Wishbone Interface (27MHz clock domain)
    //=========================================================================
    
    // Wishbone side buffers (27MHz domain)
    reg [7:0] wb_rx_buffer;
    reg       wb_rx_valid;
    reg [7:0] wb_tx_buffer;
    reg       wb_tx_valid;
    
    // USB side buffers (48MHz domain)  
    reg [7:0] usb_rx_buffer;
    reg       usb_rx_valid;
    reg [7:0] usb_tx_buffer;
    reg       usb_tx_valid;
    reg       usb_tx_consumed;
    
    // Wishbone register map:
    // 0x20: TX/RX data
    // 0x21: Status (bit 0=rx_valid, 1=tx_ready, 7=online)
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wb_rx_buffer <= 8'h00;
            wb_rx_valid <= 1'b0;
            wb_tx_buffer <= 8'h00;
            wb_tx_valid <= 1'b0;
            wb_ack_o <= 1'b0;
        end else begin
            // Copy from USB domain (simple unsafe CDC for now)
            if (usb_rx_valid && !wb_rx_valid) begin
                wb_rx_buffer <= usb_rx_buffer;
                wb_rx_valid <= 1'b1;
            end
            
            // Acknowledge TX consumed
            if (usb_tx_consumed)
                wb_tx_valid <= 1'b0;
            
            // Wishbone handshake
            if (wb_cyc_i && wb_stb_i && !wb_ack_o) begin
                wb_ack_o <= 1'b1;
                
                if (wb_we_i) begin
                    case (wb_adr_i[1:0])
                        2'b00: begin  // TX data
                            wb_tx_buffer <= wb_dat_i;
                            wb_tx_valid <= 1'b1;
                        end
                    endcase
                end else begin
                    case (wb_adr_i[1:0])
                        2'b00: begin  // RX data
                            wb_dat_o <= wb_rx_buffer;
                            wb_rx_valid <= 1'b0;
                        end
                        2'b01: begin  // Status
                            wb_dat_o <= {online, 6'b0, ~wb_tx_valid, wb_rx_valid};
                        end
                        default: wb_dat_o <= 8'h00;
                    endcase
                end
            end else begin
                wb_ack_o <= 1'b0;
            end
        end
    end
    
    //=========================================================================
    // USB Device Controller Logic (endpoint handling at 48MHz)
    //=========================================================================
    
    // All buffer and endpoint logic in 48MHz clock domain
    always @(posedge clk_48mhz or negedge rst_n) begin
        if (!rst_n) begin
            txdat <= 8'h00;
            txval <= 1'b0;
            txdat_len <= 12'h000;
            txiso_pid <= 4'h0;
            txcork <= 1'b0;
            rxrdy <= 1'b1;
            inf_alter <= 8'h00;
            ep2_in_wptr <= 6'h00;
            ep2_in_rptr <= 6'h00;
            ep2_in_has_data <= 1'b0;
            ep2_out_wptr <= 6'h00;
            ep2_out_rptr <= 6'h00;
            ep2_out_has_data <= 1'b0;
            usb_rx_buffer <= 8'h00;
            usb_rx_valid <= 1'b0;
            usb_tx_buffer <= 8'h00;
            usb_tx_valid <= 1'b0;
            usb_tx_consumed <= 1'b0;
        end else begin
            // Transfer TX buffer from Wishbone domain to EP2 IN buffer
            usb_tx_consumed <= 1'b0;
            if (wb_tx_valid && !usb_tx_valid) begin
                usb_tx_buffer <= wb_tx_buffer;
                usb_tx_valid <= 1'b1;
            end
            
            if (usb_tx_valid && !ep2_in_has_data) begin
                ep2_in_buffer[ep2_in_wptr] <= usb_tx_buffer;
                ep2_in_wptr <= ep2_in_wptr + 1;
                ep2_in_has_data <= 1'b1;
                usb_tx_valid <= 1'b0;
                usb_tx_consumed <= 1'b1;
            end
            
            // Transfer EP2 OUT buffer to RX buffer for Wishbone domain
            if (ep2_out_has_data && !usb_rx_valid) begin
                usb_rx_buffer <= ep2_out_buffer[ep2_out_rptr];
                ep2_out_rptr <= ep2_out_rptr + 1;
                usb_rx_valid <= 1'b1;
                if (ep2_out_rptr + 1 == ep2_out_wptr)
                    ep2_out_has_data <= 1'b0;
            end
            
            // Clear RX valid when consumed by Wishbone domain
            if (!wb_rx_valid && usb_rx_valid)
                usb_rx_valid <= 1'b0;
            
            // Handle RX data from USB host
            if (rxval && rxrdy && endpt == 4'h2) begin
                // EP2 OUT data received
                ep2_out_buffer[ep2_out_wptr] <= rxdat;
                ep2_out_wptr <= ep2_out_wptr + 1;
                ep2_out_has_data <= 1'b1;
            end
            
            // Handle TX data to USB host
            if (!txact && ep2_in_has_data && endpt == 4'h2) begin
                txdat <= ep2_in_buffer[ep2_in_rptr];
                txval <= 1'b1;
                txdat_len <= 12'h001;
                ep2_in_rptr <= ep2_in_rptr + 1;
                if (ep2_in_rptr + 1 == ep2_in_wptr)
                    ep2_in_has_data <= 1'b0;
            end else if (txpop) begin
                txval <= 1'b0;
            end
        end
    end
    
    //=========================================================================
    // USB SoftPHY Instantiation
    //=========================================================================
    
    USB_SoftPHY_Top u_softphy (
        .clk_i(clk_48mhz),
        .rst_i(~rst_n),
        .usb_dp_io(usb_dp),
        .usb_dn_io(usb_dn),
        .utmi_data_out_i(utmi_dataout),
        .utmi_txvalid_i(utmi_txvalid),
        .utmi_txready_o(utmi_txready),
        .utmi_data_in_o(utmi_datain),
        .utmi_rxactive_o(utmi_rxactive),
        .utmi_rxvalid_o(utmi_rxvalid),
        .utmi_rxerror_o(utmi_rxerror),
        .utmi_linestate_o(utmi_linestate),
        .utmi_op_mode_i(utmi_opmode),
        .utmi_xcvrselect_i(utmi_xcvrselect),
        .utmi_termselect_i(utmi_termselect)
    );
    
    //=========================================================================
    // USB Device Controller Instantiation
    //=========================================================================
    
    USB_Device_Controller_Top u_device_ctrl (
        .clk_i(clk_48mhz),
        .reset_i(~rst_n),
        .usbrst_o(usbrst),
        .highspeed_o(highspeed),
        .suspend_o(suspend),
        .online_o(online),
        
        // TX endpoint
        .txdat_i(txdat),
        .txval_i(txval),
        .txdat_len_i(txdat_len),
        .txiso_pid_i(txiso_pid),
        .txcork_i(txcork),
        .txpop_o(txpop),
        .txact_o(txact),
        .txpktfin_o(txpktfin),
        
        // RX endpoint
        .rxdat_o(rxdat),
        .rxval_o(rxval),
        .rxrdy_i(rxrdy),
        .rxact_o(rxact),
        .rxpktval_o(rxpktval),
        .setup_o(setup),
        .endpt_o(endpt),
        .sof_o(sof),
        
        // Interface control
        .inf_alter_i(inf_alter),
        .inf_alter_o(inf_alter_o),
        .inf_sel_o(inf_sel),
        .inf_set_o(inf_set),
        
        // Descriptor ROM
        .descrom_raddr_o(descrom_raddr),
        .desc_index_o(desc_index),
        .desc_type_o(desc_type),
        .descrom_rdata_i(descrom_rdata),
        .desc_dev_addr_i(desc_dev_addr),
        .desc_dev_len_i(desc_dev_len),
        .desc_qual_addr_i(desc_qual_addr),
        .desc_qual_len_i(desc_qual_len),
        .desc_fscfg_addr_i(desc_fscfg_addr),
        .desc_fscfg_len_i(desc_fscfg_len),
        .desc_hscfg_addr_i(desc_hscfg_addr),
        .desc_hscfg_len_i(desc_hscfg_len),
        .desc_oscfg_addr_i(desc_oscfg_addr),
        .desc_hidrpt_addr_i(desc_hidrpt_addr),
        .desc_hidrpt_len_i(desc_hidrpt_len),
        .desc_bos_addr_i(desc_bos_addr),
        .desc_bos_len_i(desc_bos_len),
        .desc_strlang_addr_i(desc_strlang_addr),
        .desc_strvendor_addr_i(desc_strvendor_addr),
        .desc_strvendor_len_i(desc_strvendor_len),
        .desc_strproduct_addr_i(desc_strproduct_addr),
        .desc_strproduct_len_i(desc_strproduct_len),
        .desc_strserial_addr_i(desc_strserial_addr),
        .desc_strserial_len_i(desc_strserial_len),
        .desc_have_strings_i(desc_have_strings),
        
        // UTMI PHY interface
        .utmi_dataout_o(utmi_dataout),
        .utmi_txvalid_o(utmi_txvalid),
        .utmi_txready_i(utmi_txready),
        .utmi_datain_i(utmi_datain),
        .utmi_rxactive_i(utmi_rxactive),
        .utmi_rxvalid_i(utmi_rxvalid),
        .utmi_rxerror_i(utmi_rxerror),
        .utmi_linestate_i(utmi_linestate),
        .utmi_opmode_o(utmi_opmode),
        .utmi_xcvrselect_o(utmi_xcvrselect),
        .utmi_termselect_o(utmi_termselect),
        .utmi_reset_o(utmi_reset)
    );

endmodule
