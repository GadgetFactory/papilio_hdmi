/*
 * usb_serial_no2.v
 *
 * Wishbone wrapper for no2usb core with CDC-ACM serial
 * Simpler integration than TinyFPGA
 */

module usb_serial_no2 (
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
    output wire       wb_ack_o
);

    // no2usb core uses 12-bit addressing for wishbone
    // We'll map our 8-bit address space to it:
    // 0x20 -> 0x000 (control/status register)
    // 0x21 -> 0x004 (data register)
    // etc.
    
    wire [11:0] usb_wb_addr;
    wire [15:0] usb_wb_rdata;
    wire [15:0] usb_wb_wdata;
    wire usb_wb_we;
    wire usb_wb_cyc;
    wire usb_wb_ack;
    
    // Map our 8-bit Wishbone to no2usb's 16-bit Wishbone
    assign usb_wb_addr = {4'b0000, wb_adr_i};  // Just use lower address bits
    assign usb_wb_wdata = {8'h00, wb_dat_i};   // Extend to 16-bit
    assign usb_wb_we = wb_we_i;
    assign usb_wb_cyc = wb_cyc_i && wb_stb_i;
    assign wb_ack_o = usb_wb_ack;
    
    // Extract lower 8 bits from USB core's 16-bit data
    always @(*) begin
        wb_dat_o = usb_wb_rdata[7:0];
    end
    
    // no2usb USB core
    // This core has integrated Wishbone interface!
    usb #(
        .TARGET("GENERIC"),  // Use generic instead of ICE40
        .EPDW(8),            // 8-bit endpoint data width
        .EVT_DEPTH(0),       // No event FIFO
        .IRQ(0)              // No IRQ
    ) usb_core (
        // USB pads
        .pad_dp(usb_dp),
        .pad_dn(usb_dn),
        .pad_pu(),           // Pull-up control (leave floating)
        
        // EP buffer interface (not used directly in Wishbone mode)
        .ep_tx_addr_0(9'h000),
        .ep_tx_data_0(8'h00),
        .ep_tx_we_0(1'b0),
        .ep_rx_addr_0(9'h000),
        .ep_rx_data_1(),
        .ep_rx_re_0(1'b0),
        .ep_clk(clk),
        
        // Wishbone interface
        .wb_addr(usb_wb_addr),
        .wb_rdata(usb_wb_rdata),
        .wb_wdata(usb_wb_wdata),
        .wb_we(usb_wb_we),
        .wb_cyc(usb_wb_cyc),
        .wb_ack(usb_wb_ack),
        
        // IRQ (not used)
        .irq(),
        
        // SOF (not used)
        .sof(),
        
        // Clock (48MHz)
        .clk(clk_48mhz),
        .rst(!rst_n)
    );

endmodule
