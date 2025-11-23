// char_ram_8x8.v
// Character RAM and ROM font for 80x30 text mode (640x480 @ 8x16 chars)
// Port for video scan (read-only) and Wishbone interface (read/write)

module char_ram_8x8 (
    // Video scan interface
    input wire v_clk,
    input wire v_en,
    input wire [11:0] v_addr,  // 80x30 = 2400 addresses (12 bits)
    output reg [7:0] v_char,   // Character code
    output reg [7:0] v_attr,   // Attribute (foreground/background color)
    
    // Wishbone interface for MCU writes
    input wire wb_clk,
    input wire [11:0] wb_addr,
    input wire [7:0] wb_dat_i,
    output reg [7:0] wb_dat_o,
    input wire wb_we,
    input wire wb_en,
    input wire wb_addr_sel  // 0=char, 1=attr
);

    // Dual-port RAM for character codes (2400 bytes for 80x30)
    reg [7:0] char_ram [0:2399];
    
    // Dual-port RAM for attributes (2400 bytes for 80x30)
    reg [7:0] attr_ram [0:2399];
    
    // Initialize character RAM with spaces
    integer i;
    initial begin
        for (i = 0; i < 2400; i = i + 1) begin
            char_ram[i] = 8'h20;  // Space character
            attr_ram[i] = 8'h07;  // White on black
        end
    end
    
    // Video scan read port
    always @(posedge v_clk) begin
        if (v_en && v_addr < 2400) begin
            v_char <= char_ram[v_addr];
            v_attr <= attr_ram[v_addr];
        end
    end
    
    // Wishbone write/read port
    always @(posedge wb_clk) begin
        if (wb_en && wb_addr < 2400) begin
            if (wb_we) begin
                if (wb_addr_sel == 0)
                    char_ram[wb_addr] <= wb_dat_i;
                else
                    attr_ram[wb_addr] <= wb_dat_i;
            end
            wb_dat_o <= wb_addr_sel ? attr_ram[wb_addr] : char_ram[wb_addr];
        end
    end

endmodule


// font_rom_8x8.v
// 8x8 font ROM containing standard ASCII characters
module font_rom_8x8 (
    input wire clk,
    input wire [7:0] char_code,
    input wire [2:0] row,
    output reg [7:0] pixels
);

    // Font ROM - 256 characters x 8 rows x 8 pixels
    // Using standard VGA 8x8 font
    reg [7:0] font_mem [0:2047];
    
    initial begin
        $readmemh("font_8x8.hex", font_mem);
    end
    
    wire [10:0] addr = {char_code, row};
    
    always @(posedge clk) begin
        pixels <= font_mem[addr];
    end

endmodule
