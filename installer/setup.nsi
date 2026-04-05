; NSIS Script for Quant Platform

!include "MUI2.nsh"

Name "Quant Platform"
OutFile "QuantPlatform_Setup.exe"
InstallDir "$PROGRAMFILES64\QuantPlatform"
RequestExecutionLevel admin

; UI settings
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetOutPath "$INSTDIR"
    File "..\build_msvc\Release\QuantPlatform.exe"

    SetOutPath "$INSTDIR\config"
    File /r "..\config\*.*"

    SetOutPath "$INSTDIR\data"
    File /r "..\data\*.*"

    ; Create shortcuts
    CreateDirectory "$SMPROGRAMS\Quant Platform"
    CreateShortcut "$SMPROGRAMS\Quant Platform\Quant Platform.lnk" "$INSTDIR\QuantPlatform.exe"
    CreateShortcut "$SMPROGRAMS\Quant Platform\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortcut "$DESKTOP\Quant Platform.lnk" "$INSTDIR\QuantPlatform.exe"

    ; Write uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\QuantPlatform.exe"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR\config"
    RMDir /r "$INSTDIR\data"
    RMDir "$INSTDIR"
    Delete "$SMPROGRAMS\Quant Platform\*.lnk"
    RMDir "$SMPROGRAMS\Quant Platform"
    Delete "$DESKTOP\Quant Platform.lnk"
SectionEnd
