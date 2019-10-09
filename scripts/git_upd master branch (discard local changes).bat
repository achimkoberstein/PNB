set branch=master

echo **************************************************************
echo ATTENTION: All your local changes (in tracked not commited files) and commits will be overwritten!
echo **************************************************************

set coin_root=%~dp0..\
cd %coin_root%

set /p answer="Continue (y/n): "

if %answer%==y (git fetch --all)
if %answer%==y (git reset --hard origin/%branch%)

timeout 20