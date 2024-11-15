$AppName = "sayonara"
$SourcePath = ".\$AppName.exe"
$InstallPath = "C:\Program Files\$AppName"
$RegPath = "HKLM\Software\Classes\*\shell\sayonara"
$RegCommandPath = "$RegPath\command"
$Command = "`"$InstallPath\$AppName.exe`""
$Input_ = "`"%1`""

# Ensure PowerShell is running as Administrator
If (-Not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "Please run this script as an Administrator."
	
	Write-Host "`nPress ENTER to close..."
	Read-Host
	
    Exit
}

# Check if the source file exists
If (-Not (Test-Path $SourcePath)) {
    Write-Host "Executable not found. Compile the application first."
	
	Write-Host "`nPress ENTER to close..."
	Read-Host
	
    Exit
}

# Create the installation folder if it doesn't exist
If (-Not (Test-Path $InstallPath)) {
    Write-Host "Creating installation directory at $InstallPath..."
    New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
}

# Copy the application to the system-wide installation path
Write-Host "Installing $AppName to $InstallPath..."
Copy-Item -Path $SourcePath -Destination $InstallPath -Force

# Use reg.exe to create the registry keys and set values
Write-Host "Adding context menu entry to the registry using reg.exe..."
& reg.exe ADD "$RegPath" /ve /d "$AppName" /f
& reg.exe ADD "$RegCommandPath" /ve /d "`"$Command`" `"$Input_`"" /f

Write-Host "Installation complete! You should now see '$AppName' in the right-click context menu."

Write-Host "`nPress ENTER to close..."
Read-Host

# TODO! ask to verify overwrite if exists for reg + file
# TODO display path to tool
