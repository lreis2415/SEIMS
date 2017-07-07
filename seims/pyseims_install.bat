REM
REM    This file is aimed for Packages checking and installing for SEIMS on Windows platform.
REM
REM    SEIMS is distributed for Research and/or Education only,
REM    any commercial purpose will be FORBIDDEN.
REM    SEIMS is an open-source project,
REM    but WITHOUT ANY WARRANTY; WITHOUT even the implied warranty of
REM    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
REM
REM    See the GNU General Public License for more details.
REM

pushd %~dp0
cd %~dp0
cd pygeoc
rd /s/q dist
REM Compile and install PyGeoC through pip
pip install tox
python setup.py bdist_wheel --python-tag py2
cd dist
for /f "delims=" %%i in ('dir /s/b "*.whl"') do (
echo installing %%~ni ...
pip install %%i --upgrade
)
cd ../..
REM Install all dependent packages required by SEIMS
for /r . %%i in ("*requirements.txt") do (
echo installing %%~ni ...
pip install -r %%i
)
pause