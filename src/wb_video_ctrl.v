// wb_video_ctrl.v
// Wishbone wrapper for HDMI video output control
// Slave 1: Base address 0x10-0x1F

module wb_video_ctrl (
    input wire clk,
    input wire rst_n,
    
    // Wishbone interface
    input wire [7:0] wb_adr_i,
    input wire [7:0] wb_dat_i,
    output reg [7:0] wb_dat_o,
    input wire wb_cyc_i,
    input wire wb_stb_i,
    input wire wb_we_i,
    output reg wb_ack_o,
    
    // HDMI outputs
    output wire O_tmds_clk_p,
    output wire O_tmds_clk_n,
    output wire [2:0] O_tmds_data_p,
    output wire [2:0] O_tmds_data_n
);

    // Wishbone control registers
    reg [7:0] pattern_mode;  // Address 0x10: Pattern mode selection
    
    // Wishbone bus handling
    wire wb_valid = wb_cyc_i & wb_stb_i;
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wb_ack_o <= 0;
            wb_dat_o <= 8'h00;
            pattern_mode <= 8'h00;  // Default: color bars
        end else begin
            wb_ack_o <= 0;
            
            if (wb_valid && !wb_ack_o) begin
                wb_ack_o <= 1;
                
                if (wb_we_i) begin
                    // Write operations
                    case (wb_adr_i[3:0])
                        4'h0: pattern_mode <= wb_dat_i;
                        default: ;
                    endcase
                end else begin
                    // Read operations
                    case (wb_adr_i[3:0])
                        4'h0: wb_dat_o <= pattern_mode;
                        4'h1: wb_dat_o <= 8'h01;  // Version/status
                        default: wb_dat_o <= 8'h00;
                    endcase
                end
            end
        end
    end
    
    // Instantiate video_top with pattern mode control
    video_top_wb u_video (
        .I_clk(clk),
        .I_rst_n(rst_n),
        .I_pattern_mode(pattern_mode[1:0]),  // Use lower 2 bits for 4 patterns
        .O_tmds_clk_p(O_tmds_clk_p),
        .O_tmds_clk_n(O_tmds_clk_n),
        .O_tmds_data_p(O_tmds_data_p),
        .O_tmds_data_n(O_tmds_data_n)
    );

endmodule
