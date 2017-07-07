REM
REM    This file is aimed for reCompile and reInstall PyGeoC for debugging on Windows platform.
REM

cd %~dp0
rd /s/q dist
pip install tox
python setup.py bdist_wheel --python-tag py2
cd dist
for /f "delims=" %%i in ('dir /s/b "*.whl"') do (
echo installing %%~ni ...
pip install %%i --upgrade
)