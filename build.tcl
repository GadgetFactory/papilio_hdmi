# Gowin build script
set_device GW2A-LV18PG256C8/I7
add_file src/dvi_tx/dvi_tx.v
add_file src/gowin_rpll/TMDS_rPLL.v
add_file src/simple_spi_wb_bridge_debug.v
add_file src/testpattern.v
add_file src/top.v
add_file src/uart_tx.v
add_file src/usb_serial_gowin.v
add_file src/video_top.v
add_file src/video_top_wb.v
add_file src/wb_address_decoder.v
add_file src/wb_rgb_led_ctrl.v
add_file src/wb_simple_rgb_led.v
add_file src/wb_video_ctrl.v
add_file src/pins.cst
add_file src/timing.sdc
run all
