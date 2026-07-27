; Inno Setup Script - MiHobbyC
; Requiere Inno Setup 6.0+

[Setup]
AppName=MiHobbyC
AppVersion=2.1
AppPublisher=MiHobbyC
DefaultDirName={autopf}\MiHobbyC
DefaultGroupName=MiHobbyC
OutputDir=installer
OutputBaseFilename=MiHobbyC-Setup
Compression=lzma2/ultra64
SolidCompression=yes
UninstallDisplayIcon={app}\MiHobbyC.ico
SetupIconFile=MiHobbyC.ico
LicenseFile=LICENSE
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "bin\Debug\MiHobbyC.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "MiHobbyC.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\MiHobbyC"; Filename: "{app}\MiHobbyC.exe"; IconFilename: "{app}\MiHobbyC.ico"
Name: "{group}\Desinstalar MiHobbyC"; Filename: "{uninstallexe}"
Name: "{autodesktop}\MiHobbyC"; Filename: "{app}\MiHobbyC.exe"; IconFilename: "{app}\MiHobbyC.ico"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Crear icono en el escritorio"; GroupDescription: "Iconos adicionales:"

[Run]
Filename: "{app}\MiHobbyC.exe"; Description: "Ejecutar MiHobbyC ahora"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{localappdata}\MiHobbyC"
