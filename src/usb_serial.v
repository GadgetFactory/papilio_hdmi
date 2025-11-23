// usb_serial_wishbone.v
// USB CDC-ACM Serial Port with Wishbone Interface
// This is a wrapper that will integrate with an external USB core
// For full implementation, use: https://github.com/no2fpga/no2usb
// or TinyFPGA bootloader USB stack

module usb_serial_wishbone (
    input wire clk,            // System clock (27MHz)
    input wire clk_48mhz,      // 48MHz clock for USB (needs PLL)
    input wire rst_n,
    
    // USB pins
    inout wire usb_dp,         // USB D+ at G11
    inout wire usb_dn,         // USB D- at H12
    output wire usb_pu,        // USB pullup control (optional)
    
    // Wishbone interface
    input wire [7:0] wb_adr_i,
    input wire [7:0] wb_dat_i,
    output reg [7:0] wb_dat_o,
    input wire wb_cyc_i,
    input wire wb_stb_i,
    input wire wb_we_i,
    output reg wb_ack_o
);

    // Wishbone registers
    // 0x00: TX data (write) / RX data (read)
    // 0x01: Status (bit 0 = rx_valid, bit 1 = tx_ready, bit 7 = usb_connected)
    // 0x02: Control (bit 0 = enable)
    
    reg [7:0] tx_data_reg;
    reg [7:0] rx_data_reg;
    reg tx_valid;
    reg rx_ready;
    reg usb_enable;
    
    // USB signals
    wire usb_tx_ready;
    wire usb_rx_valid;
    wire [7:0] usb_rx_data;
    wire usb_connected;
    
    // USB pullup - enable when configured
    assign usb_pu = usb_enable;
    
    // Wishbone bus handling
    wire wb_valid = wb_cyc_i & wb_stb_i;
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wb_ack_o <= 0;
            wb_dat_o <= 8'h00;
            tx_data_reg <= 8'h00;
            tx_valid <= 0;
            rx_ready <= 1;
            usb_enable <= 1;  // Enable USB by default
        end else begin
            wb_ack_o <= 0;
            tx_valid <= 0;
            
            // Capture received data
            if (usb_rx_valid && rx_ready) begin
                rx_data_reg <= usb_rx_data;
                rx_ready <= 0;  // Wait for read
            end
            
            if (wb_valid && !wb_ack_o) begin
                wb_ack_o <= 1;
                
                if (wb_we_i) begin
                    // Write operations
                    case (wb_adr_i[1:0])
                        2'h0: begin  // TX data
                            tx_data_reg <= wb_dat_i;
                            tx_valid <= 1;
                        end
                        2'h2: begin  // Control
                            usb_enable <= wb_dat_i[0];
                        end
                        default: ;
                    endcase
                end else begin
                    // Read operations
                    case (wb_adr_i[1:0])
                        2'h0: begin  // RX data
                            wb_dat_o <= rx_data_reg;
                            rx_ready <= 1;  // Mark as read
                        end
                        2'h1: begin  // Status
                            wb_dat_o <= {usb_connected, 5'b0, usb_tx_ready, !rx_ready};
                        end
                        2'h2: begin  // Control readback
                            wb_dat_o <= {7'b0, usb_enable};
                        end
                        default: wb_dat_o <= 8'h00;
                    endcase
                end
            end
        end
    end
    
    // TinyFPGA USB Core Integration
    // Uses the TinyFPGA bootloader USB stack for CDC-ACM serial
    wire [6:0] dev_addr;
    
    // USB PHY signals
    wire usb_p_tx, usb_n_tx, usb_p_rx, usb_n_rx, usb_tx_en;
    
    // Bidirectional buffer control
    assign usb_dp = usb_tx_en ? usb_p_tx : 1'bz;
    assign usb_dn = usb_tx_en ? usb_n_tx : 1'bz;
    assign usb_p_rx = usb_dp;
    assign usb_n_rx = usb_dn;
    
    // Control endpoint signals
    wire ctrl_out_ep_req, ctrl_out_ep_grant, ctrl_out_ep_data_avail;
    wire ctrl_out_ep_setup, ctrl_out_ep_data_get, ctrl_out_ep_acked;
    wire [7:0] ctrl_out_ep_data;
    wire ctrl_in_ep_req, ctrl_in_ep_grant, ctrl_in_ep_data_free;
    wire ctrl_in_ep_data_put, ctrl_in_ep_data_done, ctrl_in_ep_acked;
    wire [7:0] ctrl_in_ep_data;
    
    // Serial endpoint signals  
    wire serial_out_ep_req, serial_out_ep_grant, serial_out_ep_data_avail;
    wire serial_out_ep_data_get, serial_out_ep_acked;
    wire [7:0] serial_out_ep_data;
    wire serial_in_ep_req, serial_in_ep_grant, serial_in_ep_data_free;
    wire serial_in_ep_data_put, serial_in_ep_data_done, serial_in_ep_acked;
    wire [7:0] serial_in_ep_data;
    
    // Tie-off wires for unused setup/stall signals
    wire serial_out_ep_setup_tie;
    assign serial_out_ep_setup_tie = 1'b0;
    
    // Create stall buses
    wire [1:0] out_ep_stall_bus, in_ep_stall_bus;
    assign out_ep_stall_bus = 2'b00;
    assign in_ep_stall_bus = 2'b00;
    
    // USB Protocol Engine
    usb_fs_pe #(
        .NUM_OUT_EPS(2),  // Control + Serial OUT
        .NUM_IN_EPS(2)    // Control + Serial IN
    ) usb_pe (
        .clk_48mhz(clk_48mhz),
        .clk(clk),
        .reset(!rst_n),
        .usb_p_tx(usb_p_tx),
        .usb_n_tx(usb_n_tx),
        .usb_p_rx(usb_p_rx),
        .usb_n_rx(usb_n_rx),
        .usb_tx_en(usb_tx_en),
        .dev_addr(dev_addr),
        
        // OUT endpoints
        .out_ep_req({serial_out_ep_req, ctrl_out_ep_req}),
        .out_ep_grant({serial_out_ep_grant, ctrl_out_ep_grant}),
        .out_ep_data_avail({serial_out_ep_data_avail, ctrl_out_ep_data_avail}),
        .out_ep_setup({serial_out_ep_setup_tie, ctrl_out_ep_setup}),
        .out_ep_data_get({serial_out_ep_data_get, ctrl_out_ep_data_get}),
        .out_ep_data(serial_out_ep_data),  // Shared data bus
        .out_ep_stall(out_ep_stall_bus),
        .out_ep_acked({serial_out_ep_acked, ctrl_out_ep_acked}),
        
        // IN endpoints
        .in_ep_req({serial_in_ep_req, ctrl_in_ep_req}),
        .in_ep_grant({serial_in_ep_grant, ctrl_in_ep_grant}),
        .in_ep_data_free({serial_in_ep_data_free, ctrl_in_ep_data_free}),
        .in_ep_data_put({serial_in_ep_data_put, ctrl_in_ep_data_put}),
        .in_ep_data({serial_in_ep_data, ctrl_in_ep_data}),
        .in_ep_data_done({serial_in_ep_data_done, ctrl_in_ep_data_done}),
        .in_ep_stall(in_ep_stall_bus),
        .in_ep_acked({serial_in_ep_acked, ctrl_in_ep_acked}),
        
        .sof_valid(),
        .frame_index()
    );
    
    // USB Serial Control Endpoint (handles enumeration and CDC-ACM)
    usb_serial_ctrl_ep ctrl_ep (
        .clk(clk),
        .reset(!rst_n),
        .dev_addr(dev_addr),
        
        // OUT endpoint
        .out_ep_req(ctrl_out_ep_req),
        .out_ep_grant(ctrl_out_ep_grant),
        .out_ep_data_avail(ctrl_out_ep_data_avail),
        .out_ep_setup(ctrl_out_ep_setup),
        .out_ep_data_get(ctrl_out_ep_data_get),
        .out_ep_data(serial_out_ep_data),
        .out_ep_stall(),
        .out_ep_acked(ctrl_out_ep_acked),
        
        // IN endpoint
        .in_ep_req(ctrl_in_ep_req),
        .in_ep_grant(ctrl_in_ep_grant),
        .in_ep_data_free(ctrl_in_ep_data_free),
        .in_ep_data_put(ctrl_in_ep_data_put),
        .in_ep_data(ctrl_in_ep_data),
        .in_ep_data_done(ctrl_in_ep_data_done),
        .in_ep_stall(),
        .in_ep_acked(ctrl_in_ep_acked)
    );
    
    // Serial data endpoint bridge
    // OUT endpoint (host to device - RX)
    assign serial_out_ep_req = 1'b1;  // Always request data
    assign usb_rx_valid = serial_out_ep_data_avail;
    assign usb_rx_data = serial_out_ep_data;
    assign serial_out_ep_data_get = rx_ready && serial_out_ep_data_avail;
    
    // IN endpoint (device to host - TX)
    assign serial_in_ep_req = tx_valid;
    assign serial_in_ep_data = tx_data_reg;
    assign serial_in_ep_data_put = tx_valid && serial_in_ep_data_free;
    assign serial_in_ep_data_done = serial_in_ep_data_put;
    assign usb_tx_ready = serial_in_ep_data_free;
    
    // USB connected when enumerated
    assign usb_connected = (dev_addr != 7'd0);

endmodule
