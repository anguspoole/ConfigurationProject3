# ConfigurationProject3
This project uses MSBUILD and NSIS to install a game and create an uninstaller, along with validating license/login info

By: Angus Poole

## Video
This project's set up is more detailed than most others, so I suggest just watching the video.

https://www.youtube.com/watch?v=IOvVSEelmjU

## Part 1: MSBUILD INFO
In order to Zip up files, the MS Build Community Tasks Project was downloaded and installed.
Found here: https://github.com/loresoft/msbuildtasks
The initial project02.zip file will be sent to C:/cnd/PooleAngus/videogame/, as per project 2

NOTE: This program MUST be built/compiled in Debug/x86 mode!

Build the project using the msbuild command.

## Part 2: INSTALL/UNINSTALL INFO
To extract Zip files, the ZipDLL plug-in for NSIS was used. It can be found here: https://nsis.sourceforge.io/ZipDLL_plug-in
After running the installer, the extracted files will be found in C:/cnd/poole_angus/project03/
The uninstaller will also be found in C:/cnd/poole_angus/project03/
The shortcut created will be called videogame
The uninstaller and installer are set to use x64 based registry (with SetRegView 64)

## Part 3: LICENSE INFO
Acceptable Logins:
1. Email: angus@fanshaweonline.ca 	Password: p001e
2. Email: admin@fanshaweonline.ca	Password: pass!word0
3. Email: oscar@fanshaweonline.ca	Password: 0sc4r

To validate the license, the installer will attempt to reach the server and access validatelicense.php
The server was setup through xampp. This php file must be placed in the htdocs/nsi/ folder of xampp.
The installer will place a file, license.txt, in the C:/cnd/poole_angus/project03/ folder.
The exe uses libCURL to access the same server, and will compare the license found to the license generated by the installer.
The exe uses the console for logging in.
If login fails due to bad credentials/invalid license, then a message will be displayed and ESC can be pressed to quit.
Otherwise, if the login succeeds, the program will run as normal.