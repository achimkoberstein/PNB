@echo off

echo **************************************************************
echo Current branch to be updated from the remote server (origin).
echo In case of conflicts the opperation aborts.
echo **************************************************************

set coin_root=%~dp0..\
cd %coin_root%

git stash 
git pull
git stash pop

timeout 20