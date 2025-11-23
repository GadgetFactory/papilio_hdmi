// wb_text_mode.v
// Text mode video controller with 80x30 character display
// Uses 8x8 font for 640x480 output (actually 640x240 with 2x vertical scaling)

module wb_text_mode (
    input wire clk,
    input wire clk_pixel,
    input wire rst_n,
    
    // Wishbone interface
    input wire [7:0] wb_adr_i,
    input wire [7:0] wb_dat_i,
    output reg [7:0] wb_dat_o,
    input wire wb_cyc_i,
    input wire wb_stb_i,
    input wire wb_we_i,
    output reg wb_ack_o,
    
    // Video timing inputs (from hdmi_timing)
    input wire video_active,
    input wire [9:0] pixel_x,
    input wire [9:0] pixel_y,
    
    // RGB output
    output reg [7:0] red,
    output reg [7:0] green,
    output reg [7:0] blue
);

    // Character RAM (80x30 = 2400 chars)
    wire [11:0] char_addr;
    wire [7:0] char_code;
    wire [7:0] char_attr;
    
    // Font ROM
    wire [7:0] font_pixels;
    wire [2:0] font_row;
    
    // Calculate character position from pixel position
    wire [6:0] char_x = pixel_x[9:3];  // 0-79 (divide by 8)
    wire [4:0] char_y = pixel_y[8:4];  // 0-29 (divide by 16, 2x vertical)
    wire [2:0] pixel_in_char_x = pixel_x[2:0];
    wire [2:0] pixel_in_char_y = pixel_y[3:1];  // Use [3:1] for 2x vertical scaling
    
    assign char_addr = (char_y * 80) + char_x;
    assign font_row = pixel_in_char_y;
    
    // Character RAM instance
    char_ram_8x8 char_ram_inst (
        .v_clk(clk_pixel),
        .v_en(video_active && char_x < 80 && char_y < 30),
        .v_addr(char_addr),
        .v_char(char_code),
        .v_attr(char_attr),
        
        .wb_clk(clk),
        .wb_addr(wb_adr_i[11:0]),
        .wb_dat_i(wb_dat_i),
        .wb_dat_o(wb_dat_o),
        .wb_we(wb_we_i && wb_cyc_i && wb_stb_i),
        .wb_en(wb_cyc_i && wb_stb_i),
        .wb_addr_sel(wb_adr_i[12])  // bit 12: 0=char, 1=attr
    );
    
    // Font ROM instance
    font_rom_8x8 font_rom_inst (
        .clk(clk_pixel),
        .char_code(char_code),
        .row(font_row),
        .pixels(font_pixels)
    );
    
    // Wishbone acknowledge
    always @(posedge clk) begin
        wb_ack_o <= wb_cyc_i && wb_stb_i && !wb_ack_o;
    end
    
    // Pixel generation - extract the appropriate bit from font data
    reg pixel_on;
    always @(posedge clk_pixel) begin
        if (video_active && char_x < 80 && char_y < 30) begin
            pixel_on <= font_pixels[7 - pixel_in_char_x];
        end else begin
            pixel_on <= 1'b0;
        end
    end
    
    // Color generation from attribute byte
    // Attribute format: [7:4] = background color, [3:0] = foreground color
    // Color format (4-bit): [3]=bright, [2]=red, [1]=green, [0]=blue
    wire [3:0] fg_color = char_attr[3:0];
    wire [3:0] bg_color = char_attr[7:4];
    wire [3:0] active_color = pixel_on ? fg_color : bg_color;
    
    always @(posedge clk_pixel) begin
        if (video_active) begin
            // Convert 4-bit color to 8-bit RGB
            red   <= {active_color[2], active_color[3], 6'b0} | (active_color[2] ? 8'h3F : 8'h00);
            green <= {active_color[1], active_color[3], 6'b0} | (active_color[1] ? 8'h3F : 8'h00);
            blue  <= {active_color[0], active_color[3], 6'b0} | (active_color[0] ? 8'h3F : 8'h00);
        end else begin
            red   <= 8'h00;
            green <= 8'h00;
            blue  <= 8'h00;
        end
    end

endmodule
