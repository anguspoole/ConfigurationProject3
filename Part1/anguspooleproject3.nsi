!include "nsDialogs.nsh"
!include "LogicLib.nsh"
!include "ZipDLL.nsh"

#installer name
OutFile "AngusPooleProject3Installer.exe"
Name "Angus Poole Project 3 Installer"

RequestExecutionLevel admin

XPStyle on

LicenseText "License"
LicenseData "./gpl.txt"

InstallDir "c:\cnd\poole_angus\project_03"

!define VALID_LICENSE "CNDP3POOLEANGUS"

Var Dialog
Var Label
Var Label0
Var Text_State
Var Checkbox
Var Checkbox_State
Var inputFile
var UserEmailText
var UserEmail_State
var PasswordText
var Password_State

Page custom nsDialogPage nsDialogPageLeave 
Page custom nsOptionals nsOptionalsLeave
Page custom nsUserPassPage nsUserPassPageLeave
Page License 
Page Instfiles 
UninstPage instfiles

#Installer

Function .onInit
    SetRegView 64
    StrCpy $Text_State "Installer..."
    SetOutPath "$INSTDIR"
FunctionEnd

Section "Install"
    # Uninstaller
	writeUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Function nsDialogPage
nsDialogs::Create 1018
Pop $Dialog
${If} $Dialog == error
Abort
${EndIf}

${NSD_CreateLabel} 0 0 100% 12u "Welcome!"
Pop $Label

${NSD_CreateLabel} 0 13u 100% 12u "This is the installer for DistanceDemonstrater - by Poole, Angus "
Pop $Label0

nsDialogs::Show
FunctionEnd

Function nsDialogPageLeave
FunctionEnd

Function nsUserPassPage
nsDialogs::Create 1018
Pop $Dialog
${If} $Dialog == error
Abort
${EndIf}

${NSD_CreateLabel} 0 0 100% 12u "User Email:"
Pop $Label

${NSD_CreateText} 0 14u 100% 12u "Type email here"
Pop $UserEmailText

${NSD_CreateLabel} 0 32u 100% 12u "Password:"
Pop $Label0

${NSD_CreatePassword} 0 45u 100% 12u ""
Pop $PasswordText

nsDialogs::Show
FunctionEnd

Function nsUserPassPageLeave
${NSD_GetText} $UserEmailText $UserEmail_State
${NSD_GetText} $PasswordText $Password_State
MessageBox MB_OK "You typed: $UserEmail_State and Password: $Password_State"

inetc::post "email=$UserEmail_State&pass=$Password_State" "http://localhost/nsi/validatelicense.php" "$EXEDIR\license.txt"

Pop $0 #return value = exit code, "OK" if OK
MessageBox MB_OK "Download status: $0"

#open license file
FileOpen $0 "$EXEDIR\license.txt" r
FileRead $0 $1
FileClose $0

StrCmp $1 ${VALID_LICENSE} 0 +3
MessageBox MB_OK "License is valid!"
Goto +3
MessageBox MB_OK "INVALID LICENSE - cancelling install!"
Quit
FunctionEnd

Function nsOptionals

nsDialogs::Create 1018

${NSD_CreateCheckbox} 0 0u 100% 10u "Include the extra_assets folder"
Pop $Checkbox

${NSD_SetState} $Checkbox $Checkbox_State

${If} $Checkbox_State == ${BST_CHECKED}
    ${NSD_Check} $Checkbox
${EndIf}

nsDialogs::Show

FunctionEnd

Function nsOptionalsLeave

${NSD_GetState} $Checkbox $Checkbox_State

FunctionEnd

Section MyPOST

    StrCpy $inputFile "c:\cnd\PooleAngus\project02.zip"
    ZipDLL::extractall $inputFile $INSTDIR

    CopyFiles "$EXEDIR\license.txt" "$INSTDIR\license.txt"

    #remove the extra assets if the box was not checked
    ${If} $Checkbox_State == ${BST_UNCHECKED}
        RMDir /r /REBOOTOK "$INSTDIR\extra_assets"
    ${EndIf}

    CreateShortcut "$SMPROGRAMS\videogame.lnk" "$INSTDIR\GDP1819FeeneyGL.exe" "" "" 0

    WriteRegExpandStr HKLM "SOFTWARE\INFO6025\POOLEANGUS\" "videogame" "$INSTDIR\GDP1819FeeneyGL.exe"
SectionEnd

#Uninstaller

Function un.onInit
  # your code here
  SetRegView 64
FunctionEnd

Section Uninstall

    SetRegView 64

    #delete shortcut
    delete "$SMPROGRAMS\videogame.lnk"

    #i. Delete “c:\cnd\YourLastNameYourFirstName” folder
    RMDir /r /REBOOTOK "c:\cnd\PooleAngus\"

    #ii. Remove previously created registry entry
    DeleteRegValue HKLM "SOFTWARE\INFO6025\POOLEANGUS\" "videogame"
    #DeleteRegKey HKLM "SOFTWARE\INFO6025\POOLEANGUS"

    #iii. No files related to your install should remain on host computer

SectionEnd