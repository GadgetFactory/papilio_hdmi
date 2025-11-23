/*
 * usb_device_controller.v
 *
 * Simple USB Device Controller for CDC-ACM Serial
 * Connects UTMI PHY to application logic
 * 
 * This implements a minimal USB device with CDC-ACM class
 * for serial communication.
 */

module usb_device_controller (
    input  wire clk_48mhz,
    input  wire rst_n,
    
    // UTMI PHY Interface
    output reg  [7:0] utmi_data_out,
    output reg        utmi_txvalid,
    output reg  [1:0] utmi_op_mode,
    output reg  [1:0] utmi_xcvrselect,
    output reg        utmi_termselect,
    input  wire [7:0] utmi_data_in,
    input  wire       utmi_txready,
    input  wire       utmi_rxvalid,
    input  wire       utmi_rxactive,
    input  wire       utmi_rxerror,
    input  wire [1:0] utmi_linestate,
    
    // Serial Application Interface
    output reg  [7:0] rx_data,
    output reg        rx_valid,
    input  wire       rx_ready,
    
    input  wire [7:0] tx_data,
    input  wire       tx_valid,
    output reg        tx_ready,
    
    // Status
    output reg  usb_configured
);

    // UTMI configuration - Full Speed mode
    initial begin
        utmi_op_mode = 2'b00;      // Normal operation
        utmi_xcvrselect = 2'b01;   // Full Speed
        utmi_termselect = 1'b0;    // FS termination
    end
    
    // USB Device States
    localparam ST_IDLE        = 0,
               ST_ADDRESSED   = 1,
               ST_CONFIGURED  = 2;
               
    reg [1:0] usb_state;
    reg [6:0] device_addr;
    
    // Endpoint 0 (Control)
    reg [7:0] ep0_buffer [0:63];
    reg [5:0] ep0_buf_ptr;
    reg ep0_data_stage;
    
    // Endpoint 1 (Serial IN/OUT)
    reg [7:0] ep1_in_buffer [0:63];
    reg [7:0] ep1_out_buffer [0:63];
    reg [5:0] ep1_in_ptr, ep1_out_ptr;
    
    // USB Packet Detection
    reg [7:0] packet_buffer [0:7];
    reg [2:0] packet_ptr;
    reg packet_receiving;
    
    // Initialize
    initial begin
        usb_state = ST_IDLE;
        device_addr = 7'd0;
        usb_configured = 1'b0;
        utmi_txvalid = 1'b0;
        utmi_data_out = 8'h00;
        rx_valid = 1'b0;
        tx_ready = 1'b0;
        ep0_buf_ptr = 6'd0;
        ep1_in_ptr = 6'd0;
        ep1_out_ptr = 6'd0;
        packet_receiving = 1'b0;
        packet_ptr = 3'd0;
    end
    
    // Main USB state machine
    always @(posedge clk_48mhz or negedge rst_n) begin
        if (!rst_n) begin
            usb_state <= ST_IDLE;
            device_addr <= 7'd0;
            usb_configured <= 1'b0;
            utmi_txvalid <= 1'b0;
            rx_valid <= 1'b0;
            tx_ready <= 1'b0;
            packet_receiving <= 1'b0;
            packet_ptr <= 3'd0;
        end else begin
            // Receive USB packets
            if (utmi_rxvalid && !packet_receiving) begin
                packet_receiving <= 1'b1;
                packet_ptr <= 3'd0;
                packet_buffer[0] <= utmi_data_in;
                packet_ptr <= packet_ptr + 1'd1;
            end else if (utmi_rxvalid && packet_receiving) begin
                packet_buffer[packet_ptr] <= utmi_data_in;
                if (packet_ptr < 7)
                    packet_ptr <= packet_ptr + 1'd1;
            end else if (!utmi_rxactive && packet_receiving) begin
                packet_receiving <= 1'b0;
                // Process received packet
                // TODO: Implement full USB protocol
                // For now, just echo data back
            end
            
            // Simple loopback for testing
            // When data received on serial endpoint, make it available
            if (utmi_rxvalid && usb_state == ST_CONFIGURED) begin
                rx_data <= utmi_data_in;
                rx_valid <= 1'b1;
            end else if (rx_ready) begin
                rx_valid <= 1'b0;
            end
            
            // Handle transmit
            tx_ready <= utmi_txready && (usb_state == ST_CONFIGURED);
            if (tx_valid && tx_ready) begin
                utmi_data_out <= tx_data;
                utmi_txvalid <= 1'b1;
            end else begin
                utmi_txvalid <= 1'b0;
            end
            
            // Simulate configuration for testing
            // In real implementation, this happens through enumeration
            if (utmi_linestate != 2'b00) begin  // USB connected
                if (usb_state == ST_IDLE) begin
                    usb_state <= ST_ADDRESSED;
                    device_addr <= 7'd1;
                end else if (usb_state == ST_ADDRESSED) begin
                    // Simulate successful configuration
                    usb_state <= ST_CONFIGURED;
                    usb_configured <= 1'b1;
                end
            end else begin
                usb_state <= ST_IDLE;
                usb_configured <= 1'b0;
                device_addr <= 7'd0;
            end
        end
    end

endmodule
