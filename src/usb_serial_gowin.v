/*
 * usb_serial_gowin.v
 *
 * Wishbone wrapper for Gowin USB SoftPHY + Device Controller
 * Now with actual USB implementation!
 */

module usb_serial_gowin (
    // Clock and Reset
    input  wire clk,
    input  wire clk_48mhz,
    input  wire rst_n,
    
    // USB PHY
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

    // USB PHY signals (UTMI interface)
    wire [7:0] utmi_data_out;
    wire       utmi_txvalid;
    wire [1:0] utmi_op_mode;
    wire [1:0] utmi_xcvrselect;
    wire       utmi_termselect;
    wire [7:0] utmi_data_in;
    wire       utmi_txready;
    wire       utmi_rxvalid;
    wire       utmi_rxactive;
    wire       utmi_rxerror;
    wire [1:0] utmi_linestate;
    
    // Application interface
    wire [7:0] usb_rx_data;
    wire       usb_rx_valid;
    reg        usb_rx_ready;
    
    wire [7:0] usb_tx_data_out;  // Renamed to avoid conflict
    wire       usb_tx_valid_out;
    wire       usb_tx_ready;
    
    wire       usb_configured;
    
    // Internal registers for Wishbone interface
    reg [7:0] rx_buffer;
    reg rx_buffer_valid;
    reg [7:0] tx_buffer;
    reg tx_buffer_valid;
    
    // TX signals going to USB controller
    reg [7:0] usb_tx_data;
    reg       usb_tx_valid;
    
    // Wishbone register map:
    // 0x20: TX/RX data
    // 0x21: Status (bit 0=rx_valid, 1=tx_ready, 7=usb_connected)
    // 0x22: Control
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            rx_buffer <= 8'h00;
            rx_buffer_valid <= 1'b0;
            tx_buffer <= 8'h00;
            tx_buffer_valid <= 1'b0;
            wb_ack_o <= 1'b0;
            usb_rx_ready <= 1'b0;
            usb_tx_data <= 8'h00;
            usb_tx_valid <= 1'b0;
        end else begin
            // Wishbone bus handshake
            if (wb_cyc_i && wb_stb_i && !wb_ack_o) begin
                wb_ack_o <= 1'b1;
                
                if (wb_we_i) begin
                    // Write
                    case (wb_adr_i[1:0])
                        2'b00: begin  // 0x20 - TX data
                            tx_buffer <= wb_dat_i;
                            tx_buffer_valid <= 1'b1;
                        end
                        default: ;
                    endcase
                end else begin
                    // Read
                    case (wb_adr_i[1:0])
                        2'b00: begin  // 0x20 - RX data
                            wb_dat_o <= rx_buffer;
                            rx_buffer_valid <= 1'b0;  // Clear on read
                        end
                        2'b01: begin  // 0x21 - Status
                            wb_dat_o <= {usb_configured, 6'b0, ~tx_buffer_valid, rx_buffer_valid};
                        end
                        default: wb_dat_o <= 8'h00;
                    endcase
                end
            end else begin
                wb_ack_o <= 1'b0;
            end
            
            // USB RX: Store incoming data from USB
            if (usb_rx_valid && !rx_buffer_valid) begin
                rx_buffer <= usb_rx_data;
                rx_buffer_valid <= 1'b1;
                usb_rx_ready <= 1'b1;
            end else begin
                usb_rx_ready <= 1'b0;
            end
            
            // USB TX: Send buffered data to USB
            if (tx_buffer_valid && usb_tx_ready) begin
                usb_tx_data <= tx_buffer;
                usb_tx_valid <= 1'b1;
                tx_buffer_valid <= 1'b0;
            end else begin
                usb_tx_valid <= 1'b0;
            end
        end
    end
    
    // Instantiate Gowin USB SoftPHY (UTMI PHY)
    USB_SoftPHY_Top u_usb_phy (
        .clk_i(clk_48mhz),
        .rst_i(~rst_n),
        
        // UTMI Interface
        .utmi_data_out_i(utmi_data_out),
        .utmi_txvalid_i(utmi_txvalid),
        .utmi_op_mode_i(utmi_op_mode),
        .utmi_xcvrselect_i(utmi_xcvrselect),
        .utmi_termselect_i(utmi_termselect),
        .utmi_data_in_o(utmi_data_in),
        .utmi_txready_o(utmi_txready),
        .utmi_rxvalid_o(utmi_rxvalid),
        .utmi_rxactive_o(utmi_rxactive),
        .utmi_rxerror_o(utmi_rxerror),
        .utmi_linestate_o(utmi_linestate),
        
        // USB Pins
        .usb_dp_io(usb_dp),
        .usb_dn_io(usb_dn)
    );
    
    // Instantiate USB Device Controller
    usb_device_controller u_usb_dev (
        .clk_48mhz(clk_48mhz),
        .rst_n(rst_n),
        
        // UTMI PHY Interface
        .utmi_data_out(utmi_data_out),
        .utmi_txvalid(utmi_txvalid),
        .utmi_op_mode(utmi_op_mode),
        .utmi_xcvrselect(utmi_xcvrselect),
        .utmi_termselect(utmi_termselect),
        .utmi_data_in(utmi_data_in),
        .utmi_txready(utmi_txready),
        .utmi_rxvalid(utmi_rxvalid),
        .utmi_rxactive(utmi_rxactive),
        .utmi_rxerror(utmi_rxerror),
        .utmi_linestate(utmi_linestate),
        
        // Serial Application Interface
        .rx_data(usb_rx_data),
        .rx_valid(usb_rx_valid),
        .rx_ready(usb_rx_ready),
        
        .tx_data(usb_tx_data),
        .tx_valid(usb_tx_valid),
        .tx_ready(usb_tx_ready),
        
        // Status
        .usb_configured(usb_configured)
    );

endmodule
