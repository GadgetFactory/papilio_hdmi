// video_top_wb.v
// Wrapper for video_top with Wishbone pattern mode control

module video_top_wb
(
    input             I_clk           , //27Mhz
    input             I_rst_n         ,
    input [1:0]       I_pattern_mode  , // Pattern selection from Wishbone
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
    
    // Test pattern generator with Wishbone mode control
    testpattern testpattern_inst
    (
        .I_pxl_clk   (pix_clk            ),
        .I_rst_n     (hdmi4_rst_n        ),
        .I_mode      ({1'b0, I_pattern_mode}),  // Use Wishbone pattern mode
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
    
    // DVI transmitter
    DVI_TX_Top DVI_TX_Top_inst
    (
        .I_rst_n       (hdmi4_rst_n   ),
        .I_serial_clk  (serial_clk    ),
        .I_rgb_clk     (pix_clk       ),
        .I_rgb_vs      (tp0_vs_in     ), 
        .I_rgb_hs      (tp0_hs_in     ),    
        .I_rgb_de      (tp0_de_in     ), 
        .I_rgb_r       (tp0_data_r    ),
        .I_rgb_g       (tp0_data_g    ),  
        .I_rgb_b       (tp0_data_b    ),  
        .O_tmds_clk_p  (O_tmds_clk_p  ),
        .O_tmds_clk_n  (O_tmds_clk_n  ),
        .O_tmds_data_p (O_tmds_data_p ),
        .O_tmds_data_n (O_tmds_data_n )
    );

endmodule
