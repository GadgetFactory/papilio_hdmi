// Wishbone Slave Wrapper for WS2812B RGB LED Controller
// Provides a simple register interface to control RGB LEDs
// Simplified version for single LED

module wb_rgb_led_ctrl #(
    parameter ADDR_WIDTH = 32,
    parameter DATA_WIDTH = 32
)(
    // Clock and Reset
    input wire clk,
    input wire rst,
    
    // Wishbone Slave Interface
    input wire [ADDR_WIDTH-1:0] wb_adr_i,
    input wire [DATA_WIDTH-1:0] wb_dat_i,
    output reg [DATA_WIDTH-1:0] wb_dat_o,
    input wire wb_we_i,
    input wire wb_cyc_i,
    input wire wb_stb_i,
    output reg wb_ack_o,
    input wire [3:0] wb_sel_i,
    
    // RGB LED Output
    output wire led_out
);

    // Register map
    // 0x00: Control/Status Register
    //       [0] - START (write 1 to start transmission)
    //       [1] - BUSY (read-only)
    // 0x04: LED0 RGB data [23:0] (GRB format)
    
    reg [23:0] led0_data;
    reg start;
    wire busy;
    
    // Address decode
    wire addr_ctrl = (wb_adr_i[7:0] == 8'h00);
    wire addr_led0 = (wb_adr_i[7:0] == 8'h04);
    
    // Wishbone read/write with simple logic
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            wb_ack_o <= 1'b0;
            wb_dat_o <= 32'h0;
            start <= 1'b0;
            led0_data <= 24'h000000;  // Off initially
        end else begin
            // Default: clear one-shot signals
            wb_ack_o <= 1'b0;
            start <= 1'b0;
            
            // Wishbone operation
            if (wb_cyc_i && wb_stb_i) begin
                wb_ack_o <= 1'b1;
                
                if (wb_we_i) begin
                    // Write
                    if (addr_ctrl) begin
                        start <= wb_dat_i[0];
                    end else if (addr_led0) begin
                        led0_data <= wb_dat_i[23:0];
                    end
                end else begin
                    // Read
                    if (addr_ctrl) begin
                        wb_dat_o <= {30'h0, busy, 1'b0};
                    end else if (addr_led0) begin
                        wb_dat_o <= {8'h0, led0_data};
                    end else begin
                        wb_dat_o <= 32'h0;
                    end
                end
            end
        end
    end
    
    // WS2812B controller
    ws2812b_controller #(
        .CLOCK_FREQ(27000000)
    ) ws2812b_inst (
        .clk(clk),
        .rst(rst),
        .start(start),
        .busy(busy),
        .led_data(led0_data),
        .led_out(led_out)
    );

endmodule


// WS2812B LED Controller
// Generates timing-accurate signals for WS2812B RGB LEDs
// Single LED version
module ws2812b_controller #(
    parameter CLOCK_FREQ = 27000000
)(
    input wire clk,
    input wire rst,
    input wire start,
    output reg busy,
    input wire [23:0] led_data,
    output reg led_out
);

    // WS2812B timing (800kHz bit rate)
    // T0H: 0.4us, T0L: 0.85us (total 1.25us)
    // T1H: 0.8us, T1L: 0.45us (total 1.25us)
    localparam CYCLES_PER_BIT = CLOCK_FREQ / 800000;
    localparam CYCLES_T0H = (CLOCK_FREQ * 4) / 10000000;  // 0.4us
    localparam CYCLES_T1H = (CLOCK_FREQ * 8) / 10000000;  // 0.8us
    
    reg [15:0] cycle_counter;
    reg [4:0] bit_index;
    reg current_bit;
    
    localparam IDLE = 0;
    localparam SEND_BIT = 1;
    localparam RESET = 2;
    
    reg [1:0] state;
    
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            state <= IDLE;
            busy <= 1'b0;
            led_out <= 1'b0;
            cycle_counter <= 0;
            bit_index <= 0;
            current_bit <= 1'b0;
        end else begin
            case (state)
                IDLE: begin
                    led_out <= 1'b0;
                    if (start) begin
                        busy <= 1'b1;
                        state <= SEND_BIT;
                        cycle_counter <= 0;
                        bit_index <= 23;  // Start with MSB (G7)
                        current_bit <= led_data[23];
                    end
                end
                
                SEND_BIT: begin
                    if (cycle_counter == 0) begin
                        // Start of bit period
                        led_out <= 1'b1;
                        current_bit <= led_data[bit_index];
                    end else if (cycle_counter == CYCLES_T0H && !current_bit) begin
                        // End of T0H for bit 0
                        led_out <= 1'b0;
                    end else if (cycle_counter == CYCLES_T1H && current_bit) begin
                        // End of T1H for bit 1
                        led_out <= 1'b0;
                    end
                    
                    if (cycle_counter >= CYCLES_PER_BIT - 1) begin
                        cycle_counter <= 0;
                        
                        // Move to next bit
                        if (bit_index == 0) begin
                            // All bits sent, go to reset
                            state <= RESET;
                        end else begin
                            bit_index <= bit_index - 1;
                        end
                    end else begin
                        cycle_counter <= cycle_counter + 1;
                    end
                end
                
                RESET: begin
                    // WS2812B needs >50us low reset time
                    led_out <= 1'b0;
                    if (cycle_counter >= (CLOCK_FREQ / 10000)) begin  // 100us
                        cycle_counter <= 0;
                        state <= IDLE;
                        busy <= 1'b0;
                    end else begin
                        cycle_counter <= cycle_counter + 1;
                    end
                end
            endcase
        end
    end

endmodule
