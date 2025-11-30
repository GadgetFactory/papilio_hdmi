// ==============================================================================
// wb_video_framebuffer.v - Wishbone Framebuffer Video Generator
// ==============================================================================
// Standalone 160x120 RGB332 framebuffer with Wishbone interface.
// Scaled 6x to 960x720 and centered in 1280x720 (160px black borders).
//
// This module can be instantiated independently - just connect it to the
// shared HDMI PHY layer (hdmi_phy_720p.v).
//
// Wishbone Interface:
//   Address range: 0x0000 - 0x4AFF (19,200 pixels)
//   Uses word-aligned addressing for HQVGA compatibility:
//     Pixel at (x,y) = address (y*160 + x) * 4
//   Each pixel is 8-bit RGB332: RRRGGGBB
//
// Memory: 19,200 bytes for 160x120 framebuffer
// Output: Scaled to 960x720, centered in 1280x720 (160px black borders)
//
// Usage: Instantiate this module and hdmi_phy_720p, connect RGB outputs
//        from this module to the PHY's RGB inputs.
// ==============================================================================

module wb_video_framebuffer
(
    // Wishbone slave interface
    input             I_wb_clk        ,
    input             I_wb_rst        ,
    input      [14:0] I_wb_adr        , // 15-bit for word-aligned addressing
    input      [7:0]  I_wb_dat        ,
    input             I_wb_we         ,
    input             I_wb_stb        ,
    input             I_wb_cyc        ,
    output reg        O_wb_ack        ,
    output reg [7:0]  O_wb_dat        ,
    
    // Video timing inputs (from HDMI PHY)
    input             I_pix_clk       ,
    input             I_rst_n         ,
    input      [11:0] I_h_cnt         ,
    input      [11:0] I_v_cnt         ,
    input      [11:0] I_active_x      ,
    input      [11:0] I_active_y      ,
    input             I_de            ,
    input             I_hs            ,
    input             I_vs            ,
    
    // RGB output (directly to HDMI PHY)
    output reg [7:0]  O_rgb_r         ,
    output reg [7:0]  O_rgb_g         ,
    output reg [7:0]  O_rgb_b         ,
    output reg        O_rgb_de        ,
    output reg        O_rgb_hs        ,
    output reg        O_rgb_vs        
);

// ==============================================================================
// Parameters
// ==============================================================================
localparam H_SYNC     = 12'd40;
localparam H_BPORCH   = 12'd220;
localparam H_ACTIVE   = 12'd1280;
localparam V_SYNC     = 12'd5;
localparam V_BPORCH   = 12'd20;
localparam V_ACTIVE   = 12'd720;

localparam FB_WIDTH   = 160;
localparam FB_HEIGHT  = 120;
localparam FB_SIZE    = FB_WIDTH * FB_HEIGHT;  // 19,200 pixels

localparam SCALE      = 6;
localparam SCALED_W   = FB_WIDTH * SCALE;      // 960
localparam SCALED_H   = FB_HEIGHT * SCALE;     // 720

// Framebuffer region in active area
localparam FB_X_START = (H_ACTIVE - SCALED_W) / 2;  // 160
localparam FB_X_END   = FB_X_START + SCALED_W;       // 1120

// ==============================================================================
// Framebuffer Memory - Simple Dual Port
// ==============================================================================
// Write port: Wishbone clock domain
// Read port: Pixel clock domain
// Note: Wishbone readback not supported to keep as simple dual-port RAM

(* ram_style = "block" *)
reg [7:0] framebuffer [0:FB_SIZE-1];

// Note: RAM is not pre-initialized - firmware should clear it on startup
// Gowin synthesis doesn't support large initialization loops

// ==============================================================================
// Wishbone Interface (Write Only)
// ==============================================================================
wire wb_valid = I_wb_stb && I_wb_cyc;
// HQVGA-compatible word-aligned addressing (pixel * 4)
wire [14:0] wb_pixel_addr = I_wb_adr[14:2];

// Wishbone write
always @(posedge I_wb_clk) begin
    if (wb_valid && I_wb_we && wb_pixel_addr < FB_SIZE) begin
        framebuffer[wb_pixel_addr] <= I_wb_dat;
    end
end

// ACK generation (no readback - simple dual port)
always @(posedge I_wb_clk or posedge I_wb_rst) begin
    if (I_wb_rst) begin
        O_wb_ack <= 1'b0;
        O_wb_dat <= 8'h00;
    end else begin
        O_wb_ack <= wb_valid;
        O_wb_dat <= 8'h00;  // No readback supported
    end
end

// ==============================================================================
// Framebuffer Display - Scaling Logic
// ==============================================================================
// We need to read pixels from the framebuffer at 1/6th the rate

reg [2:0] h_scale_cnt;
reg [2:0] v_scale_cnt;
reg [7:0] src_x;
reg [6:0] src_y;

wire in_fb_region = I_de && (I_active_x >= FB_X_START) && (I_active_x < FB_X_END);

// Horizontal scaling counter
always @(posedge I_pix_clk or negedge I_rst_n) begin
    if (!I_rst_n) begin
        h_scale_cnt <= 3'd0;
        src_x <= 8'd0;
    end else begin
        if (I_h_cnt == H_SYNC + H_BPORCH + FB_X_START - 1) begin
            h_scale_cnt <= 3'd0;
            src_x <= 8'd0;
        end else if (in_fb_region && src_x < FB_WIDTH) begin
            if (h_scale_cnt == 3'd5) begin
                h_scale_cnt <= 3'd0;
                src_x <= src_x + 1'b1;
            end else begin
                h_scale_cnt <= h_scale_cnt + 1'b1;
            end
        end
    end
end

// Vertical scaling - track line ends
reg hs_prev;
wire hs_tick = !I_hs && hs_prev;
always @(posedge I_pix_clk) hs_prev <= I_hs;

always @(posedge I_pix_clk or negedge I_rst_n) begin
    if (!I_rst_n) begin
        v_scale_cnt <= 3'd0;
        src_y <= 7'd0;
    end else begin
        if (I_v_cnt == V_SYNC + V_BPORCH - 1 && I_h_cnt == 0) begin
            v_scale_cnt <= 3'd0;
            src_y <= 7'd0;
        end else if (hs_tick && (I_v_cnt >= V_SYNC + V_BPORCH) && (I_v_cnt < V_SYNC + V_BPORCH + V_ACTIVE)) begin
            if (v_scale_cnt == 3'd5) begin
                v_scale_cnt <= 3'd0;
                if (src_y < FB_HEIGHT - 1)
                    src_y <= src_y + 1'b1;
            end else begin
                v_scale_cnt <= v_scale_cnt + 1'b1;
            end
        end
    end
end

// Calculate framebuffer address: y * 160 + x
// 160 = 128 + 32 = (1 << 7) + (1 << 5)
wire [14:0] y_times_128 = {1'b0, src_y, 7'b0};
wire [14:0] y_times_32  = {3'b0, src_y, 5'b0};
wire [14:0] fb_addr = y_times_128 + y_times_32 + {7'b0, src_x};

// ==============================================================================
// Pipeline for framebuffer read
// ==============================================================================
// Stage 1: Register address and flags
reg [14:0] fb_addr_d1;
reg in_fb_region_d1;
reg de_d1, hs_d1, vs_d1;

always @(posedge I_pix_clk) begin
    fb_addr_d1 <= fb_addr;
    in_fb_region_d1 <= in_fb_region;
    de_d1 <= I_de;
    hs_d1 <= I_hs;
    vs_d1 <= I_vs;
end

// Stage 2: Read from framebuffer
reg [7:0] pixel_data;
reg in_fb_region_d2;
reg de_d2, hs_d2, vs_d2;

always @(posedge I_pix_clk) begin
    if (in_fb_region_d1 && fb_addr_d1 < FB_SIZE)
        pixel_data <= framebuffer[fb_addr_d1];
    else
        pixel_data <= 8'd0;
    
    in_fb_region_d2 <= in_fb_region_d1;
    de_d2 <= de_d1;
    hs_d2 <= hs_d1;
    vs_d2 <= vs_d1;
end

// ==============================================================================
// RGB332 to RGB888 expansion
// ==============================================================================
wire [7:0] exp_r = {pixel_data[7:5], pixel_data[7:5], pixel_data[7:6]};
wire [7:0] exp_g = {pixel_data[4:2], pixel_data[4:2], pixel_data[4:3]};
wire [7:0] exp_b = {pixel_data[1:0], pixel_data[1:0], pixel_data[1:0], pixel_data[1:0]};

// ==============================================================================
// Output
// ==============================================================================
always @(posedge I_pix_clk or negedge I_rst_n) begin
    if (!I_rst_n) begin
        O_rgb_r <= 8'd0;
        O_rgb_g <= 8'd0;
        O_rgb_b <= 8'd0;
        O_rgb_de <= 1'b0;
        O_rgb_hs <= 1'b0;
        O_rgb_vs <= 1'b0;
    end else begin
        O_rgb_de <= de_d2;
        O_rgb_hs <= hs_d2;
        O_rgb_vs <= vs_d2;
        
        if (de_d2 && in_fb_region_d2) begin
            O_rgb_r <= exp_r;
            O_rgb_g <= exp_g;
            O_rgb_b <= exp_b;
        end else begin
            O_rgb_r <= 8'd0;
            O_rgb_g <= 8'd0;
            O_rgb_b <= 8'd0;
        end
    end
end

endmodule
