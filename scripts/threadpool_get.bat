@echo off
REM put this script into coin_root\scripts

REM Directories
set coin_root=%~dp0..\
set third_party_dir=%coin_root%ThirdParty\
set threadpool_dir=%third_party_dir%threadpool\

cd %third_party_dir%


REM **************************************************************
REM Download Threadpool to coin_root\ThirdParty/threadpool folder
REM **************************************************************
if not exist %threadpool_dir% (echo ThirdParty\threadpool not found, downloading it from the github repository.)
if not exist %threadpool_dir% (set downloaded_repo=true) else (set downloaded_repo=false)
if not exist %threadpool_dir% (call git clone --recursive -j8 https://github.com/philipphenkel/threadpool.git)
if exist %threadpool_dir% (echo ThirdParty\boost found)


cd %threadpool_dir%
REM Save commit hash
if %downloaded_repo%==true (git rev-parse HEAD > current_commit.txt)

set /p answer="Delete unnesessary for PNB folders (y/n): "
if %answer%==y (echo Deleting unnesessary for PNB folders) else (goto:end)

rmdir /s /q ".git"

:end

timeout 20

END