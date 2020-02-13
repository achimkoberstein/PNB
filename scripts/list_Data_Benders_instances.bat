@echo off
REM .sto and .stoch files are considered to be a problem instance names.
REM sometimes .cor .tim .sto where .cor != .sto != .tim are needed.
cd ../Data/Benders
ECHO Start parsing ../Data/Benders/*.sto and *.stoch files.
forfiles /s /m *.sto /c "cmd /c echo @relpath" > instance_list.txt
forfiles /s /m *.stoch /c "cmd /c echo @relpath" >> instance_list.txt
ECHO The list of files is saved to ../Data/Benders/instance_list.txt.
pause