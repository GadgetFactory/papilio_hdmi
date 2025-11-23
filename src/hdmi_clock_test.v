// hdmi_clock_test.v
// Minimal HDMI test - just output the clock signal
// Tests if the DVI_TX IP core can work with 1.8V bank

module hdmi_clock_test (
    input wire clk_27mhz,
    input wire rst,
    
    // HDMI clock output only
    output wire O_tmds_clk_p,
    output wire O_tmds_clk_n
);

    wire rst_n = ~rst;
    wire serial_clk;
    wire pll_lock;
    wire pix_clk;
    wire hdmi_rst_n;
    
    // PLL: 27MHz -> 158.625MHz serial clock
    TMDS_rPLL u_tmds_rpll (
        .clkin(clk_27mhz),
        .clkout(serial_clk),
        .lock(pll_lock)
    );
    
    assign hdmi_rst_n = rst_n & pll_lock;
    
    // Clock divider: serial_clk / 5 = pixel clock
    CLKDIV u_clkdiv (
        .RESETN(hdmi_rst_n),
        .HCLKIN(serial_clk),
        .CLKOUT(pix_clk),
        .CALIB(1'b1)
    );
    defparam u_clkdiv.DIV_MODE = "5";
    defparam u_clkdiv.GSREN = "false";
    
    // DVI TX with minimal connections - clock only
    DVI_TX_Top DVI_TX_Top_inst (
        .I_rst_n(hdmi_rst_n),
        .I_serial_clk(serial_clk),
        .I_rgb_clk(pix_clk),
        .I_rgb_vs(1'b0),
        .I_rgb_hs(1'b0),
        .I_rgb_de(1'b0),
        .I_rgb_r(8'h00),
        .I_rgb_g(8'h00),
        .I_rgb_b(8'h00),
        .O_tmds_clk_p(O_tmds_clk_p),
        .O_tmds_clk_n(O_tmds_clk_n),
        .O_tmds_data_p(),
        .O_tmds_data_n()
    );

endmodule
