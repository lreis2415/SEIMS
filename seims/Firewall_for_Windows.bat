@echo off
echo "Configure firewall for SEIMS..."
echo "-- Liangjun Zhu, zlj@lreis.ac.cn --"
pushd %~dp0
cd %~dp0
set exePath=%~dp0
for /f "delims=" %%i in ('dir /s/b "*.exe"') do (
echo %%~ni ...
netsh advfirewall firewall add rule name=%%i dir=in action=allow program=%exePath%\%%i.exe ENABLE=yes
)
echo "--- All executable programs configured succeed!"
pause