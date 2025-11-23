# download_tinyfpga_usb.ps1
# Downloads TinyFPGA USB bootloader files for integration

$RepoBase = "https://raw.githubusercontent.com/tinyfpga/TinyFPGA-Bootloader/master/common"
$DestDir = "src/tinyfpga_usb"

# Create destination directory
New-Item -ItemType Directory -Force -Path $DestDir | Out-Null

Write-Host "Downloading TinyFPGA USB files..." -ForegroundColor Green

# List of required files
$Files = @(
    "usb_fs_pe.v",
    "usb_fs_rx.v", 
    "usb_fs_tx.v",
    "usb_fs_in_pe.v",
    "usb_fs_out_pe.v",
    "usb_fs_in_arb.v",
    "usb_fs_out_arb.v",
    "usb_fs_tx_mux.v",
    "usb_serial_ctrl_ep.v"
)

foreach ($File in $Files) {
    $Url = "$RepoBase/$File"
    $Dest = Join-Path $DestDir $File
    
    Write-Host "  Downloading $File..." -NoNewline
    
    try {
        Invoke-WebRequest -Uri $Url -OutFile $Dest -ErrorAction Stop
        Write-Host " OK" -ForegroundColor Green
    } catch {
        Write-Host " FAILED" -ForegroundColor Red
        Write-Host "    Error: $_" -ForegroundColor Red
    }
}

Write-Host "`nDownload complete!" -ForegroundColor Green
Write-Host "Files saved to: $DestDir" -ForegroundColor Cyan
Write-Host "`nNext steps:" -ForegroundColor Yellow
Write-Host "1. Review the downloaded files"
Write-Host "2. Run build_gowin.ps1 to compile"
Write-Host "3. Program the FPGA and test USB enumeration"
