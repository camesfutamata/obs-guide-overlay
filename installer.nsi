; NSIS Installer Script — obs-guide-overlay
; Baixe NSIS em: https://nsis.sourceforge.io/Download
; Compile: botão direito -> Compile NSIS Script  ou  makensis installer.nsi

!include "MUI2.nsh"
!include "LogicLib.nsh"

!define PRODUCT_NAME "Guide Overlay (Regras de Composição)"
!define PRODUCT_SHORT "obs-guide-overlay"
!define PRODUCT_VERSION "1.0.1"
!define PRODUCT_PUBLISHER "Denis"
!define OBS_PLUGINS "obs-plugins\64bit"
!define OBS_DATA "data\obs-plugins"

Name "${PRODUCT_NAME}"
OutFile "${PRODUCT_SHORT}-setup-${PRODUCT_VERSION}.exe"
InstallDir "C:\Program Files\obs-studio"
InstallDirRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\obs-studio" "DisplayIcon"
RequestExecutionLevel admin
ShowInstDetails show
ShowUnInstDetails show
SetCompressor /SOLID lzma
ManifestDPIAware true

; --- Interface ---
!define MUI_ABORTWARNING
!define MUI_ICON ""
!define MUI_UNICON ""

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "English"

; --- Seções ---
Section "Plugin" SEC_PLUGIN
  SectionIn RO

  ; DLL principal
  SetOutPath "$INSTDIR\${OBS_PLUGINS}"
  File "obs-guide-overlay.dll"

  ; Arquivos de idioma
  SetOutPath "$INSTDIR\${OBS_DATA}\${PRODUCT_SHORT}\locale"
  File /r "data\locale\*"

  ; Desinstalador
  WriteUninstaller "$INSTDIR\${OBS_PLUGINS}\uninstall-${PRODUCT_SHORT}.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT}" \
    "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT}" \
    "UninstallString" "$INSTDIR\${OBS_PLUGINS}\uninstall-${PRODUCT_SHORT}.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT}" \
    "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT}" \
    "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT}" \
    "InstallLocation" "$INSTDIR"

SectionEnd

; --- Desinstalação ---
Section "Uninstall"
  Delete "$INSTDIR\${OBS_PLUGINS}\${PRODUCT_SHORT}.dll"
  RMDir /r "$INSTDIR\${OBS_DATA}\${PRODUCT_SHORT}"
  Delete "$INSTDIR\${OBS_PLUGINS}\uninstall-${PRODUCT_SHORT}.exe"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT}"
SectionEnd
