// simple_hdmi_test.v
// Very simple HDMI test - outputs solid white screen
// Simplest possible test to verify HDMI hardware

module simple_hdmi_test (
    input wire clk,           // 27MHz system clock
    input wire rst,
    
    // HDMI outputs
    output reg hdmi_clk_p,
    output reg hdmi_clk_n,
    output reg hdmi_d0_p,
    output reg hdmi_d0_n,
    output reg hdmi_d1_p,
    output reg hdmi_d1_n,
    output reg hdmi_d2_p,
    output reg hdmi_d2_n
);

    // Simple clock divider to create pixel clock
    // 27MHz / 2 = 13.5MHz (not exactly 25.175MHz but should work for testing)
    reg clk_div;
    always @(posedge clk) begin
        clk_div <= ~clk_div;
    end
    
    // 640x480 timing counters
    reg [9:0] h_count;
    reg [9:0] v_count;
    
    // Timing parameters
    localparam H_ACTIVE = 640;
    localparam H_TOTAL = 800;
    localparam V_ACTIVE = 480;
    localparam V_TOTAL = 525;
    
    localparam H_SYNC_START = 640 + 16;
    localparam H_SYNC_END = 640 + 16 + 96;
    localparam V_SYNC_START = 480 + 10;
    localparam V_SYNC_END = 480 + 10 + 2;
    
    wire hsync = ~((h_count >= H_SYNC_START) && (h_count < H_SYNC_END));
    wire vsync = ~((v_count >= V_SYNC_START) && (v_count < V_SYNC_END));
    wire video_active = (h_count < H_ACTIVE) && (v_count < V_ACTIVE);
    
    always @(posedge clk_div or posedge rst) begin
        if (rst) begin
            h_count <= 0;
            v_count <= 0;
        end else begin
            if (h_count == H_TOTAL - 1) begin
                h_count <= 0;
                if (v_count == V_TOTAL - 1)
                    v_count <= 0;
                else
                    v_count <= v_count + 1;
            end else begin
                h_count <= h_count + 1;
            end
        end
    end
    
    // Generate simple output - white during active video, sync signals during blanking
    // For DVI/HDMI, we need to send control codes during blanking
    // During video: just send the pixel data directly (simplified, no TMDS encoding for now)
    // During blanking: toggle to create sync pattern
    
    reg [7:0] red, green, blue;
    
    always @(posedge clk_div) begin
        if (video_active) begin
            // White color during active video
            red <= 8'hFF;
            green <= 8'hFF;
            blue <= 8'hFF;
        end else begin
            // Black during blanking
            red <= 8'h00;
            green <= 8'h00;
            blue <= 8'h00;
        end
    end
    
    // Output data directly (this is NOT proper HDMI/DVI, just for basic testing)
    // In real HDMI, you need TMDS encoding and 10x serialization
    always @(posedge clk_div) begin
        // Channel 0 (Blue) - also carries hsync/vsync
        if (video_active) begin
            hdmi_d0_p <= blue[7];
            hdmi_d0_n <= ~blue[7];
        end else begin
            hdmi_d0_p <= hsync;
            hdmi_d0_n <= ~hsync;
        end
        
        // Channel 1 (Green)
        if (video_active) begin
            hdmi_d1_p <= green[7];
            hdmi_d1_n <= ~green[7];
        end else begin
            hdmi_d1_p <= vsync;
            hdmi_d1_n <= ~vsync;
        end
        
        // Channel 2 (Red)
        hdmi_d2_p <= video_active ? red[7] : 1'b0;
        hdmi_d2_n <= video_active ? ~red[7] : 1'b1;
        
        // Clock channel - toggle at pixel rate
        hdmi_clk_p <= clk_div;
        hdmi_clk_n <= ~clk_div;
    end
    
endmodule
