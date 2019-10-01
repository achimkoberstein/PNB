@echo off
REM put this script into coin_root\scripts

REM Directories
set coin_root=%~dp0..\
set third_party_dir=%coin_root%ThirdParty\
set boost_dir=%third_party_dir%boost\
 
echo Boost bilder "Bodybuilder"
echo .
echo                 ,#####,
echo                 #_   _#
echo                 la` `al
echo                 l  u  l
echo                 \  =  /
echo                 l\___/l
echo        ___ ____/:     :\____ ___
echo      .'   `.-===-\   /-===-.`   '.
echo     /      .-"""""-.-"""""-.      \
echo    /'             =:=             '\
echo  .'  ' .:    o   -=:=-   o    :. '  `.
echo  (.'   /'. '-.....-'-.....-' .'\   '.)
echo  /' ._/   ".     --:--     ."   \_. '\
echo l  .'l      ".  ---:---  ."      l'.  l
echo l  : l       l  ---:---  l       l :  l
echo  \ : l       l_____._____l       l : /
echo  /   (       l----l------l       )   \
echo /... .l      l    l      l      l. ...\
echo l::::/''    /     l       \     ''\::::l
echo '""""      /'    .L_      `\       """"'
echo           /'-.,__/` `\__..-'\
echo          ;      /     \      ;
echo          :     /       \     l
echo          l    /         \.   l
echo          l`../           l  ,/
echo          ( _ )           l  _)
echo          l   l           l   l
echo          l___l           \___l
echo          :===l            l==l
echo           \  /            l__l
echo           /\/\           /""''8.__
echo           lool           \__.//___)
echo           l==l
echo           \__/
echo Art by Joan Stark
echo .

cd %third_party_dir%

REM **************************************************************
REM Download Boost with its submudules (subfolders) to coin_root\ThirdParty/boost folder
REM **************************************************************
if not exist %boost_dir% (echo ThirdParty\boost not found, downloading boost from the github repository.)
if not exist %boost_dir% (set downloaded_repo=true) else (set downloaded_repo=false)
if not exist %boost_dir% (call git clone --recursive -j8 https://github.com/boostorg/boost.git)
if exist %boost_dir% (echo ThirdParty\boost found)

echo **************************************************************
echo Build Boost libraries that are required for PNB
echo **************************************************************
echo ThirdParty\boost\stage\lib\ with libraries and ThirdParty\bost\boost\ with include headers are to be created.

if not exist %boost_dir%b2.exe (echo Creating Boost build tools from bootstrap.bat)
if not exist %boost_dir%b2.exe (echo If bootstrap.bat fails, run it manually by double click on it!)
if not exist %boost_dir%b2.exe (cmd /C %boost_dir%bootstrap.bat)

cd %boost_dir%
REM Save commit hash
if %downloaded_repo%==true (git rev-parse HEAD > current_commit.txt)

:build_boost

echo **************************************************************
echo IMPORTANT .gitignore ignores numerous files in the ThirdParty/boost/boost. Add them using force (add ignored files) option: git add -f
echo **************************************************************

CALL :build_routine

set /p answer="Build for another Visual Studio version (known issue: run 14.2 before 11.0) (y/n): "
if %answer%==y (goto:build_boost)

set /p answer="Delete unnesessary for PNB folders (y/n): "
if %answer%==y (echo Deleting unnesessary for PNB folders.) else (goto:end)

rmdir /s /q "doc"
rmdir /s /q "more"
rmdir /s /q "status"
rmdir /s /q "tools"
rmdir /s /q ".circleci"
del "appveyor.yml" /f /q
del "b2.exe" /f /q
del "bootstrap.bat" /f /q
REM git stuff
rmdir /s /q ".git"
del ".gitattributes" /f /q
del ".gitignore" /f /q
del ".gitmodules" /f /q
del ".travis.yml" /f /q

:end

timeout 20

EXIT

:build_routine
@echo off

echo Visual Studio 2012 (Toolset 11.0, _MS_VER_ 1700) - enter 11.0
echo Visual Studio 2013 (Toolset 12.0, _MS_VER_ 1800) - enter 12.0
echo Visual Studio 2015 (Toolset 14.0, _MS_VER_ 1900) - enter 14.0
echo Visual Studio 2017 (Toolset 14.1, _MS_VER_ 1910) - enter 14.1
echo Visual Studio 2019 (Toolset 14.2, _MS_VER_ 1920) - enter 14.2
set /p answer="For which toolset to build (11.0, 12.0, 14.0, 14.1, 14.2, none <- start from higher): " 
if %answer%==none (goto:clean)

set msvcver=msvc-%answer%

REM Number of cores to use when building boost
set cores=4
 
echo Building libraries thread, system, chrono, program_options, date_time in %boost_dir% with %cores% cores using toolset %msvcver%.
 
REM Run the commands from Visual Studio Developer Command Prompt to avoid errors
REM one can use Command Prompt != Toolset if both are installed
REM 32/64 bit Command Prompt version should be used to build the 32/64 bit Boost libs, respectively

if %msvcver%==msvc-11.0 (call "%vs110comntools%vsvars32.bat") else if %msvcver%==msvc-12.0 (call "%vs120comntools%vsvars32.bat") else if %msvcver%==msvc-14.0 (call "%vs140comntools%vsvars32.bat") else if %msvcver%==msvc-14.1 (call "%vs140comntools%\VsDevCmd.bat") else if %msvcver%==msvc-14.2 (call "%programfiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat")

b2 toolset=%msvcver% link=static threading=multi runtime-link=shared --build-type=minimal --with-thread --with-system --with-chrono --with-program_options --with-date_time --abbreviate-paths architecture=x86 address-model=32 -j%cores% --stagedir=./stage --build-dir=./build_dir stage

if %msvcver%==msvc-11.0 (call "%programfiles(x86)%\Microsoft Visual Studio 11.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat") else if %msvcver%==msvc-12.0 (echo Developer, substitute this text with the call to x64 Visual Studio 2013 Dev Prompt) else if %msvcver%==msvc-14.0 (call "%programfiles(x86)%\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat") else if %msvcver%==msvc-14.1 (echo Developer, substitute this text with the call to x64 Visual Studio 2015 Dev Prompt) else if %msvcver%==msvc-14.2 (call "%programfiles(x86)%\\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat")

b2 toolset=%msvcver% link=static threading=multi runtime-link=shared --build-type=minimal --with-thread --with-system --with-chrono --with-program_options --with-date_time --abbreviate-paths architecture=x86 address-model=64 -j%cores% --stagedir=./stage --build-dir=./build_dir stage

REM Naming conventions in Boost:
REM libboost_chrono-vc110-mt-gd-x32-1_71
REM libboost - for autolinking (header files "know" automatically which file to link). For manual linking the name starts with boost
REM gd - for debugging
REM mt - multithread

:clean

echo Deleting build_dir (if any)
rmdir /s /q build_dir\
rmdir /s /q build_dir\

REM exit Visual Studio Developer Command Prompt
EXIT /B