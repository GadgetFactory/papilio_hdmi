// hdmi_gowin_ip.v
// HDMI output using Gowin IP cores (DVI_TX_Top, rPLL, CLKDIV)
// Based on working Tang Primer 20K HDMI example
// Generates 640x480@60Hz color bar pattern

module hdmi_gowin_ip (
    input wire clk_27mhz,
    input wire rst,
    
    // HDMI outputs - array notation to match working project
    output wire O_tmds_clk_p,
    output wire O_tmds_clk_n,
    output wire [2:0] O_tmds_data_p,
    output wire [2:0] O_tmds_data_n
);

    // Active low reset for IP cores
    wire rst_n = ~rst;
    
    // Clock signals
    wire serial_clk;    // High speed serial clock (5x pixel clock)
    wire pll_lock;
    wire pix_clk;       // Pixel clock
    wire hdmi_rst_n;
    
    // Video timing and data signals
    wire vs_in, hs_in, de_in;
    wire [7:0] data_r, data_g, data_b;
    
    // PLL: 27MHz input -> ~125MHz serial clock (5x 25MHz pixel clock)
    // For 640x480@60Hz we need 25.175MHz pixel clock, so serial = 125.875MHz
    // Using 27MHz * 35 / 3 / 2 = 157.5MHz serial clock
    // Then divide by 5 to get 31.5MHz pixel clock (slightly fast but will work)
    TMDS_rPLL u_tmds_rpll (
        .clkin(clk_27mhz),
        .clkout(serial_clk),
        .lock(pll_lock)
    );
    
    // Only enable HDMI when PLL is locked and reset is inactive
    assign hdmi_rst_n = rst_n & pll_lock;
    
    // Clock divider: serial_clk / 5 = pixel clock
    CLKDIV u_clkdiv (
        .RESETN(hdmi_rst_n),
        .HCLKIN(serial_clk),    // High speed clock in
        .CLKOUT(pix_clk),       // Divided clock out
        .CALIB(1'b1)
    );
    defparam u_clkdiv.DIV_MODE = "5";
    defparam u_clkdiv.GSREN = "false";
    
    // Test pattern generator - 640x480@60Hz with solid white
    testpattern testpattern_inst (
        .I_pxl_clk(pix_clk),
        .I_rst_n(hdmi_rst_n),
        .I_mode(3'b000),            // Mode 0 = color bars
        .I_single_r(8'd0),
        .I_single_g(8'd0),
        .I_single_b(8'd0),
        // 640x480@60Hz timing parameters
        .I_h_total(12'd800),        // Total horizontal pixels
        .I_h_sync(12'd96),          // Horizontal sync width
        .I_h_bporch(12'd48),        // Horizontal back porch
        .I_h_res(12'd640),          // Horizontal resolution
        .I_v_total(12'd525),        // Total vertical lines
        .I_v_sync(12'd2),           // Vertical sync width
        .I_v_bporch(12'd33),        // Vertical back porch
        .I_v_res(12'd480),          // Vertical resolution
        .I_hs_pol(1'b0),            // Horizontal sync polarity (negative)
        .I_vs_pol(1'b0),            // Vertical sync polarity (negative)
        .O_de(de_in),
        .O_hs(hs_in),
        .O_vs(vs_in),
        .O_data_r(data_r),
        .O_data_g(data_g),
        .O_data_b(data_b)
    );
    
    // DVI/HDMI transmitter IP core
    // Handles TMDS encoding and high-speed serialization
    wire [2:0] tmds_data_p, tmds_data_n;
    wire tmds_clk_p_internal, tmds_clk_n_internal;
    
    // DVI/HDMI transmitter IP core - directly connected to outputs
    DVI_TX_Top DVI_TX_Top_inst (
        .I_rst_n(hdmi_rst_n),
        .I_serial_clk(serial_clk),
        .I_rgb_clk(pix_clk),
        .I_rgb_vs(vs_in),
        .I_rgb_hs(hs_in),
        .I_rgb_de(de_in),
        .I_rgb_r(data_r),
        .I_rgb_g(data_g),
        .I_rgb_b(data_b),
        .O_tmds_clk_p(O_tmds_clk_p),
        .O_tmds_clk_n(O_tmds_clk_n),
        .O_tmds_data_p(O_tmds_data_p),
        .O_tmds_data_n(O_tmds_data_n)
    );

endmodule
