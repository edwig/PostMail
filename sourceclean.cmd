@echo off
@echo Cleaning PostMail directories
@echo .

del /q /s /f *.user
rmdir /q /s .vs
rmdir /q /s lib
rmdir /q /s BaseLibrary\x64
rmdir /q /s CryptPassword\x64
rmdir /q /s Marlin\x64
rmdir /q /s PostMail\x64
rmdir /q /s SSLSocket\x64
rmdir /q /s StyleFramework\x64

rmdir /q /s bin_Debug64
rmdir /q /s bin_DebugUnicode64
rmdir /q /s bin_Release64
rmdir /q /s bin_ReleaseUnicode64

echo .
echo Ready cleaning
