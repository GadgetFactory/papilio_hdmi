// video_top_wb.v
// Wrapper for video_top with Wishbone pattern mode control
// Mode 0-2: Test patterns, Mode 3: Text mode (80x30 character display)

module video_top_wb
(
    input             I_clk           , //27Mhz
    input             I_rst_n         ,
    input [1:0]       I_pattern_mode  , // Pattern selection from Wishbone (0-3)
    
    // Text mode character RAM interface (connected externally to wb_char_ram)
    input      [7:0]  I_text_char_data,  // Character data from RAM
    input      [7:0]  I_text_attr_data,  // Attribute data from RAM
    output reg [11:0] O_text_char_addr,  // Address to character RAM
    
    output            O_tmds_clk_p    ,
    output            O_tmds_clk_n    ,
    output     [2:0]  O_tmds_data_p   ,//{r,g,b}
    output     [2:0]  O_tmds_data_n   
);

    // Internal wires
    reg  [31:0] run_cnt;
    wire        running;
    
    wire        tp0_vs_in  ;
    wire        tp0_hs_in  ;
    wire        tp0_de_in ;
    wire [ 7:0] tp0_data_r;
    wire [ 7:0] tp0_data_g;
    wire [ 7:0] tp0_data_b;
    
    // Text mode outputs
    wire [7:0] text_data_r;
    wire [7:0] text_data_g;
    wire [7:0] text_data_b;
    
    // Video mux - select between test pattern and text mode
    wire text_mode_active = (I_pattern_mode == 2'b11);  // Mode 3
    wire [7:0] video_r = text_mode_active ? text_data_r : tp0_data_r;
    wire [7:0] video_g = text_mode_active ? text_data_g : tp0_data_g;
    wire [7:0] video_b = text_mode_active ? text_data_b : tp0_data_b;
    
    reg         vs_r;
    reg  [9:0]  cnt_vs;
    
    wire serial_clk;
    wire pll_lock;
    wire hdmi4_rst_n;
    wire pix_clk;
    
    // LED test counter
    always @(posedge I_clk or negedge I_rst_n) begin
        if(!I_rst_n)
            run_cnt <= 32'd0;
        else if(run_cnt >= 32'd27_000_000)
            run_cnt <= 32'd0;
        else
            run_cnt <= run_cnt + 1'b1;
    end
    
    assign running = (run_cnt < 32'd14_000_000) ? 1'b1 : 1'b0;
    
    // Test pattern generator with Wishbone mode control (modes 0-2)
    testpattern testpattern_inst
    (
        .I_pxl_clk   (pix_clk            ),
        .I_rst_n     (hdmi4_rst_n        ),
        .I_mode      ({1'b0, text_mode_active ? 2'b00 : I_pattern_mode}),  // Default to color bars in text mode
        .I_single_r  (8'd0               ),
        .I_single_g  (8'd255             ),
        .I_single_b  (8'd0               ),
        .I_h_total   (12'd1650           ),
        .I_h_sync    (12'd40             ),
        .I_h_bporch  (12'd220            ),
        .I_h_res     (12'd1280           ),
        .I_v_total   (12'd750            ),
        .I_v_sync    (12'd5              ),
        .I_v_bporch  (12'd20             ),
        .I_v_res     (12'd720            ),
        .I_hs_pol    (1'b1               ),
        .I_vs_pol    (1'b1               ),
        .O_de        (tp0_de_in          ),   
        .O_hs        (tp0_hs_in          ),
        .O_vs        (tp0_vs_in          ),
        .O_data_r    (tp0_data_r         ),   
        .O_data_g    (tp0_data_g         ),
        .O_data_b    (tp0_data_b         )
    );
    
    // Text mode generator (mode 3)
    // Create pixel counters for active video area (1280x720)
    reg [11:0] pixel_x;
    reg [11:0] pixel_y;
    reg        de_prev;
    
    always @(posedge pix_clk or negedge I_rst_n) begin
        if (!I_rst_n) begin
            pixel_x <= 0;
            pixel_y <= 0;
            de_prev <= 0;
        end else begin
            de_prev <= tp0_de_in;
            
            if (tp0_de_in) begin
                // During active video
                if (pixel_x < 1279)
                    pixel_x <= pixel_x + 1;
                else begin
                    pixel_x <= 0;
                    if (pixel_y < 719)
                        pixel_y <= pixel_y + 1;
                    else
                        pixel_y <= 0;
                end
            end else if (de_prev && !tp0_de_in) begin
                // End of line
                pixel_x <= 0;
            end else if (!tp0_vs_in) begin
                // During vsync, reset frame
                pixel_y <= 0;
                pixel_x <= 0;
            end
        end
    end
    
    // Calculate character position (80x30 grid, scaled for 720p)
    // 1280 pixels / 80 chars = 16 pixels per char
    // 720 lines / 30 rows = 24 lines per row (using first 30 of 45 possible)
    wire [6:0] char_x = pixel_x[10:4];  // Divide by 16 for 80 chars across 1280 pixels  
    wire [4:0] char_y = pixel_y[9:4];   // Divide by 16 for 45 lines (use top 30)
    wire [3:0] pixel_in_char_x = pixel_x[3:0];
    wire [3:0] pixel_in_char_y = pixel_y[3:0];
    
    // Character address calculation (80x30 = 2400 max)
    wire [11:0] char_addr = (char_y < 30) ? ((char_y * 80) + char_x[6:0]) : 12'd0;
    
    always @(posedge pix_clk) begin
        O_text_char_addr <= char_addr;
    end
    
    // Font ROM lookup would go here - for now, simple block display
    // This is a placeholder - proper implementation would use font_rom_8x8 module
    wire [7:0] font_row = 8'hFF;  // Placeholder: all pixels on
    wire pixel_on = (char_x < 80 && char_y < 30) ? font_row[7 - pixel_in_char_x[2:0]] : 1'b0;
    
    // Color from attribute byte (simplified)
    wire [3:0] fg_color = I_text_attr_data[3:0];
    wire [3:0] bg_color = I_text_attr_data[7:4];
    wire [3:0] active_color = pixel_on ? fg_color : bg_color;
    
    // Convert 4-bit color to 8-bit RGB
    assign text_data_r = tp0_de_in ? ({active_color[2], active_color[3], 6'b0} | (active_color[2] ? 8'h3F : 8'h00)) : 8'h00;
    assign text_data_g = tp0_de_in ? ({active_color[1], active_color[3], 6'b0} | (active_color[1] ? 8'h3F : 8'h00)) : 8'h00;
    assign text_data_b = tp0_de_in ? ({active_color[0], active_color[3], 6'b0} | (active_color[0] ? 8'h3F : 8'h00)) : 8'h00;
    
    always@(posedge pix_clk) begin
        vs_r <= tp0_vs_in;
    end
    
    always@(posedge pix_clk or negedge hdmi4_rst_n) begin
        if(!hdmi4_rst_n)
            cnt_vs <= 0;
        else if(vs_r && !tp0_vs_in)
            cnt_vs <= cnt_vs + 1'b1;
    end 
    
    // PLL for HDMI clocking
    TMDS_rPLL u_tmds_rpll
    (
        .clkin     (I_clk     ),
        .clkout    (serial_clk),
        .lock      (pll_lock  )
    );
    
    assign hdmi4_rst_n = I_rst_n & pll_lock;
    
    // Clock divider
    CLKDIV u_clkdiv
    (
        .RESETN(hdmi4_rst_n),
        .HCLKIN(serial_clk),
        .CLKOUT(pix_clk),
        .CALIB (1'b1)
    );
    defparam u_clkdiv.DIV_MODE="5";
    defparam u_clkdiv.GSREN="false";
    
    // DVI transmitter - uses muxed video output
    DVI_TX_Top DVI_TX_Top_inst
    (
        .I_rst_n       (hdmi4_rst_n   ),
        .I_serial_clk  (serial_clk    ),
        .I_rgb_clk     (pix_clk       ),
        .I_rgb_vs      (tp0_vs_in     ), 
        .I_rgb_hs      (tp0_hs_in     ),    
        .I_rgb_de      (tp0_de_in     ), 
        .I_rgb_r       (video_r       ),  // Muxed output
        .I_rgb_g       (video_g       ),  // Muxed output
        .I_rgb_b       (video_b       ),  // Muxed output
        .O_tmds_clk_p  (O_tmds_clk_p  ),
        .O_tmds_clk_n  (O_tmds_clk_n  ),
        .O_tmds_data_p (O_tmds_data_p ),
        .O_tmds_data_n (O_tmds_data_n )
    );

endmodule
