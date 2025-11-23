// Wishbone RGB LED Controller using proven simple_spi_led core
// Exposes the working 3-byte SPI protocol as Wishbone registers
// This gives you a proper Wishbone peripheral for future expansion

module wb_simple_rgb_led (
    input wire clk,
    input wire rst,
    
    // 8-bit Wishbone Slave Interface (8-bit address, 8-bit data)
    input wire [7:0] wb_adr_i,
    input wire [7:0] wb_dat_i,
    output reg [7:0] wb_dat_o,
    input wire wb_we_i,
    input wire wb_cyc_i,
    input wire wb_stb_i,
    output reg wb_ack_o,
    
    // WS2812B LED output
    output wire led_out
);

    // Color registers
    reg [7:0] led_green;
    reg [7:0] led_red;
    reg [7:0] led_blue;
    reg update_trigger;
    
    // Wishbone interface
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            led_green <= 8'h00;
            led_red <= 8'h00;
            led_blue <= 8'h00;
            update_trigger <= 0;
            wb_ack_o <= 0;
            wb_dat_o <= 8'h00;
        end else begin
            wb_ack_o <= 0;
            update_trigger <= 0;
            
            if (wb_cyc_i && wb_stb_i && !wb_ack_o) begin
                wb_ack_o <= 1;
                
                if (wb_we_i) begin
                    case (wb_adr_i[1:0])
                        2'h0: begin
                            led_green <= wb_dat_i;
                            update_trigger <= 1;  // Auto-update on any color write
                        end
                        2'h1: begin
                            led_red <= wb_dat_i;
                            update_trigger <= 1;
                        end
                        2'h2: begin
                            led_blue <= wb_dat_i;
                            update_trigger <= 1;
                        end
                    endcase
                end else begin
                    // Read
                    case (wb_adr_i[1:0])
                        2'h0: wb_dat_o <= led_green;
                        2'h1: wb_dat_o <= led_red;
                        2'h2: wb_dat_o <= led_blue;
                        2'h3: wb_dat_o <= {7'b0, busy};
                    endcase
                end
            end
        end
    end
    
    // Trigger LED update after write
    reg start;
    reg [15:0] start_delay;
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            start <= 0;
            start_delay <= 0;
        end else begin
            start <= 0;
            if (update_trigger) begin
                start_delay <= 1000;  // ~37µs delay
            end else if (start_delay > 0) begin
                start_delay <= start_delay - 1;
                if (start_delay == 1) begin
                    start <= 1;
                end
            end
        end
    end
    
    // WS2812B controller
    wire busy;
    ws2812b_controller #(
        .CLOCK_FREQ(27000000)
    ) ws2812b_inst (
        .clk(clk),
        .rst(rst),
        .start(start),
        .busy(busy),
        .led_data({led_green, led_red, led_blue}),
        .led_out(led_out)
    );

endmodule
