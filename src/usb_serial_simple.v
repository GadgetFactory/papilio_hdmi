/*
 * usb_serial_simple.v
 *
 * Ultra-simple USB CDC-ACM serial stub for Gowin FPGA
 * This is a placeholder that compiles - full implementation TBD
 * 
 * For now, this just provides the Wishbone interface and USB pins
 * without actual USB protocol implementation.
 */

module usb_serial_simple (
    // Clock and Reset
    input  wire clk_48mhz,
    input  wire clk,
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

    // USB differential buffer control
    reg usb_tx_en;
    reg usb_dp_out, usb_dn_out;
    
    // Tri-state buffers
    assign usb_dp = usb_tx_en ? usb_dp_out : 1'bz;
    assign usb_dn = usb_tx_en ? usb_dn_out : 1'bz;
    
    // Internal registers
    reg [7:0] tx_data_reg;
    reg [7:0] rx_data_reg;
    reg [7:0] status_reg;
    reg [7:0] control_reg;
    
    // Simple register access
    // Address 0x20: TX/RX data
    // Address 0x21: Status (bit 0=rx_valid, 1=tx_ready, 7=connected)
    // Address 0x22: Control (bit 0=enable)
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            tx_data_reg <= 8'h00;
            rx_data_reg <= 8'h00;
            status_reg <= 8'b00000010;  // tx_ready=1, not connected
            control_reg <= 8'h00;
            wb_ack_o <= 1'b0;
            usb_tx_en <= 1'b0;
            usb_dp_out <= 1'b0;
            usb_dn_out <= 1'b0;
        end else begin
            // Wishbone handshake
            if (wb_cyc_i && wb_stb_i && !wb_ack_o) begin
                wb_ack_o <= 1'b1;
                
                if (wb_we_i) begin
                    // Write
                    case (wb_adr_i[1:0])
                        2'b00: tx_data_reg <= wb_dat_i;  // 0x20
                        2'b01: ;  // Status is read-only
                        2'b10: control_reg <= wb_dat_i;  // 0x22
                        default: ;
                    endcase
                end else begin
                    // Read
                    case (wb_adr_i[1:0])
                        2'b00: wb_dat_o <= rx_data_reg;   // 0x20
                        2'b01: wb_dat_o <= status_reg;    // 0x21
                        2'b10: wb_dat_o <= control_reg;   // 0x22
                        default: wb_dat_o <= 8'h00;
                    endcase
                end
            end else begin
                wb_ack_o <= 1'b0;
            end
            
            // USB PHY is idle for now
            // TODO: Implement actual USB protocol
            usb_tx_en <= 1'b0;
        end
    end

endmodule
