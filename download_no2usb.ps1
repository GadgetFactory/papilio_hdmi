# Download no2usb USB core files from GitHub
# Repository: https://github.com/no2fpga/no2usb

$baseUrl = "https://raw.githubusercontent.com/no2fpga/no2usb/master/rtl"
$destDir = "src\no2usb"

# Create destination directory
New-Item -ItemType Directory -Force -Path $destDir | Out-Null

# Core RTL files needed for USB
$files = @(
    "usb.v",
    "usb_crc.v",
    "usb_ep_buf.v",
    "usb_ep_status.v",
    "usb_phy.v",
    "usb_rx_ll.v",
    "usb_rx_pkt.v",
    "usb_trans.v",
    "usb_tx_ll.v",
    "usb_tx_pkt.v",
    "usb_defs.vh"
)

Write-Host "Downloading no2usb core files..." -ForegroundColor Green

foreach ($file in $files) {
    $url = "$baseUrl/$file"
    $dest = Join-Path $destDir $file
    
    Write-Host "  Downloading $file..." -NoNewline
    try {
        Invoke-WebRequest -Uri $url -OutFile $dest -ErrorAction Stop
        Write-Host " OK" -ForegroundColor Green
    } catch {
        Write-Host " FAILED" -ForegroundColor Red
        Write-Host "    Error: $_" -ForegroundColor Red
    }
}

Write-Host "`nDownload complete!" -ForegroundColor Green
Write-Host "Files saved to: $destDir" -ForegroundColor Cyan
