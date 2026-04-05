; Inno Setup Script for Quant Platform
; Requires Inno Setup 6.x

[Setup]
AppName=Quant Platform
AppVersion=1.0.0
AppPublisher=QuantPlatform
DefaultDirName={autopf}\QuantPlatform
DefaultGroupName=Quant Platform
OutputBaseFilename=QuantPlatform_Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Files]
Source: "..\build_msvc\Release\QuantPlatform.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\config\*"; DestDir: "{app}\config"; Flags: recursesubdirs
Source: "..\data\*"; DestDir: "{app}\data"; Flags: recursesubdirs

[Icons]
Name: "{group}\Quant Platform"; Filename: "{app}\QuantPlatform.exe"
Name: "{group}\Uninstall Quant Platform"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Quant Platform"; Filename: "{app}\QuantPlatform.exe"

[Run]
Filename: "{app}\QuantPlatform.exe"; Description: "Launch Quant Platform"; Flags: nowait postinstall skipifsilent
