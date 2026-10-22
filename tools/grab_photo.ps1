# Asks the bot (running firmware/tests/cam_snap) for one photo over USB serial
# and saves it as photo_<time>.jpg in the current folder. Usage: .\tools\grab_photo.ps1 [COM3]
param([string]$Port = "COM3")

$dir = (Get-Location).Path

$sp = New-Object System.IO.Ports.SerialPort $Port, 115200
$sp.DtrEnable = $true
$sp.ReadTimeout = 10000
$sp.Open()
Start-Sleep -Milliseconds 300
$sp.DiscardInBuffer()
$sp.Write("s")

# Skip any text until the "JPEG <bytes>" header.
do { $line = $sp.ReadLine().Trim() } until ($line -match '^JPEG (\d+)$' -or $line -match '^ERROR')
if ($line -match '^ERROR') { $sp.Close(); throw "Bot says: $line" }
$len = [int]$Matches[1]

$buf = New-Object byte[] $len
$got = 0
while ($got -lt $len) { $got += $sp.Read($buf, $got, $len - $got) }
$sp.Close()

$file = Join-Path $dir ("photo_" + (Get-Date -Format "HHmmss") + ".jpg")
[System.IO.File]::WriteAllBytes($file, $buf)
Write-Output "Saved $len bytes to $((Resolve-Path $file).Path)"
