@echo off
REM .sto and .stoch files are considered to be a problem instance names.
REM sometimes .cor .tim .sto where .cor != .sto != .tim are needed.
cd ../Data/Benders
forfiles /s /m *.sto /c "cmd /c echo @relpath"
forfiles /s /m *.stoch /c "cmd /c echo @relpath"
pause