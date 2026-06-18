; Inno Setup Script — obs-guide-overlay Installer
; Baixe Inno Setup em: https://jrsoftware.org/isdl.php
; Para compilar: clique com direito neste arquivo -> Compile (ou use "Compile" no Inno Setup)

#define MyAppName "Guide Overlay (Regras de Composição)"
#define MyAppNameShort "obs-guide-overlay"
#define MyAppVersion "1.0.1"
#define MyAppPublisher "Denis"
#define MyAppURL ""

[Setup]
AppId={{B8F8A3D2-1C5E-4A9B-8D7F-6E2C3A1B9D4F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName=C:\Program Files\obs-studio\obs-plugins\64bit
DisableDirPage=no
DisableProgramGroupPage=yes
DisableStartupPrompt=yes
UninstallDisplayIcon={app}\{#MyAppNameShort}.dll
OutputDir=releases\v1.0.1
OutputBaseFilename={#MyAppNameShort}-setup-{#MyAppVersion}
Compression=lzma2/max
SolidCompression=yes
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesInstallIn64BitMode=x64compatible
ShowLanguageDialog=no
LanguageDetectionMethod=none
VersionInfoVersion={#MyAppVersion}

[Languages]
Name: "portuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
BeveledLabel=Guide Overlay

[Files]
; DLL principal → obs-plugins\64bit\
Source: "releases\v1.0.1\obs-guide-overlay.dll"; DestDir: "{app}"; Flags: ignoreversion uninsrestartdelete; Check: Is64BitInstallMode

; Arquivos de idioma → data\obs-plugins\obs-guide-overlay\locale\
Source: "data\locale\*"; DestDir: "{app}\..\..\data\obs-plugins\{#MyAppNameShort}\locale"; Flags: ignoreversion uninsrestartdelete recursesubdirs createallsubdirs

[Dirs]
Name: "{app}"
Name: "{app}\..\..\data\obs-plugins\{#MyAppNameShort}"

[UninstallDelete]
Type: filesandordirs; Name: "{app}\..\..\data\obs-plugins\{#MyAppNameShort}"

[Code]
function DetectOBSPluginsDir: string;
var
  installDir: string;
begin
  Result := '';

  // Tenta registro de desinstalação do OBS
  if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\obs-studio', 'DisplayIcon', installDir) then
  begin
    installDir := ExtractFilePath(installDir);
    if DirExists(installDir) then
    begin
      Result := installDir + '\obs-plugins\64bit';
      Exit;
    end;
  end;

  // Tenta WOW6432Node (OBS 32-bit em Windows 64-bit)
  if RegQueryStringValue(HKLM, 'SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\obs-studio', 'DisplayIcon', installDir) then
  begin
    installDir := ExtractFilePath(installDir);
    if DirExists(installDir) then
    begin
      Result := installDir + '\obs-plugins\64bit';
      Exit;
    end;
  end;

  // Tenta registro do próprio OBS Studio
  if RegQueryStringValue(HKLM, 'SOFTWARE\OBS Studio', '', installDir) then
  begin
    if DirExists(installDir) then
    begin
      Result := installDir + '\obs-plugins\64bit';
      Exit;
    end;
  end;

  // Fallback: caminho padrão
  if DirExists('C:\Program Files\obs-studio\obs-plugins\64bit') then
  begin
    Result := 'C:\Program Files\obs-studio\obs-plugins\64bit';
    Exit;
  end;

  if DirExists('C:\Program Files (x86)\obs-studio\obs-plugins\64bit') then
  begin
    Result := 'C:\Program Files (x86)\obs-studio\obs-plugins\64bit';
    Exit;
  end;
end;

procedure InitializeWizard;
var
  pluginsDir: string;
begin
  pluginsDir := DetectOBSPluginsDir;
  if pluginsDir <> '' then
    WizardForm.DirEdit.Text := pluginsDir;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if CurPageID = wpSelectDir then
  begin
    WizardForm.DirBrowseButton.Caption := 'Selecionar pasta obs-plugins\64bit...';
    if not DirExists(WizardDirValue) then
    begin
      if MsgBox('Pasta de plugins do OBS não encontrada em:' + #13#10 +
                WizardDirValue + #13#10#13#10 +
                'Deseja continuar mesmo assim? (você poderá escolher a pasta manualmente)',
                mbConfirmation, MB_YESNO) = IDNO then
        Result := False;
    end;
  end;
end;
