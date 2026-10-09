#----- Include Modern UI -----#
!include "MUI2.nsh"
!include "FileFunc.nsh"

# VERSION and SHORTSHA1 are passed in by BuildInstaller.bat (/DVERSION=...).
!ifndef VERSION
  !define VERSION "0.0.0"
!endif
!ifndef SHORTSHA1
  !define SHORTSHA1 "local"
!endif
# Directory holding the `cmake --install` output to package.
!ifndef STAGING
  !define STAGING "tmp"
!endif

Name "vgiv"
OutFile "Installvgiv-v${VERSION}-${SHORTSHA1}.exe"
BrandingText "vgiv Installer"
Icon "giv-logo.ico"
UninstallIcon "giv-logo-install.ico"

SetCompress force
CRCCheck on
XPStyle on

# Per-user install: no admin rights needed, everything goes under HKCU.
RequestExecutionLevel user
InstallDir "$LOCALAPPDATA\Programs\vgiv"
InstallDirRegKey HKCU "Software\vgiv" ""

!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\vgiv"

!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_RUN "$INSTDIR\vgiv.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Start vgiv"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "Install"
  SetOutPath "$INSTDIR"
  # vgiv.exe, runtime DLLs, and the shaders, plugins and platforms folders
  File /r "${STAGING}\*"

  CreateDirectory "$SMPROGRAMS\vgiv"
  CreateShortCut "$SMPROGRAMS\vgiv\vgiv.lnk" "$INSTDIR\vgiv.exe" "" "$INSTDIR\vgiv.exe" 0
  CreateShortCut "$SMPROGRAMS\vgiv\Uninstall vgiv.lnk" "$INSTDIR\uninst.exe"

  WriteRegStr HKCU "Software\vgiv" "" "$INSTDIR"

  # Add/Remove Programs entry
  WriteRegStr HKCU "${UNINST_KEY}" "DisplayName" "vgiv"
  WriteRegStr HKCU "${UNINST_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKCU "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninst.exe"'
  WriteRegStr HKCU "${UNINST_KEY}" "QuietUninstallString" '"$INSTDIR\uninst.exe" /S'
  WriteRegDWORD HKCU "${UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKCU "${UNINST_KEY}" "NoRepair" 1
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKCU "${UNINST_KEY}" "EstimatedSize" "$0"

  # Associate .giv files with vgiv (per-user)
  WriteRegStr HKCU "Software\Classes\.giv" "" "vgiv.GivFile"
  WriteRegStr HKCU "Software\Classes\vgiv.GivFile" "" "giv File"
  WriteRegStr HKCU "Software\Classes\vgiv.GivFile\DefaultIcon" "" "$INSTDIR\vgiv.exe,1"
  WriteRegStr HKCU "Software\Classes\vgiv.GivFile\shell" "" "open"
  WriteRegStr HKCU "Software\Classes\vgiv.GivFile\shell\open\command" "" '"$INSTDIR\vgiv.exe" "%1"'
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'

  # Pre-build the font atlas cache (~3 s per face) so the first scene opens fast
  DetailPrint "Building font cache..."
  ExecWait '"$INSTDIR\vgiv.exe" --warm-fonts'

  WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd

Section "Uninstall"
  DeleteRegKey HKCU "Software\Classes\.giv"
  DeleteRegKey HKCU "Software\Classes\vgiv.GivFile"
  DeleteRegKey HKCU "${UNINST_KEY}"
  DeleteRegKey HKCU "Software\vgiv"
  System::Call 'Shell32::SHChangeNotify(i 0x8000000, i 0, i 0, i 0)'

  RMDir /r "$SMPROGRAMS\vgiv"
  RMDir /r "$INSTDIR"
SectionEnd
