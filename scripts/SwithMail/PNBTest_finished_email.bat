@ECHO OFF
REM batch should be called as: PNBTest_finished_email.bat [test_name] [run_from_PNBTest]
REM test_name is "test" by default and will be used in email contents
REM [run_from_PNBTest] should be yes if the routine is called from NestedBendersTest.cpp

IF "%2" == "yes" (cd ..\..\..\..\scripts\SwithMail & SET test_name=%1) ELSE (SET test_name=test)

ECHO Reading credentials from email_credential.txt

setlocal enabledelayedexpansion
set /a _index=1
for /f "delims=" %%a in (email_credentials.txt) do (
  set "key!_index!=%%a"
  set /a _index+=1
  )
ECHO Credentials:
ECHO from: !key1!, email acc username: !key2!, email acc assword: !key3!, send to: !key4!

SET from=%key1%
SET username=%key2%
SET password=%key3%
SET to=%key4%
SET log_file=SwithMailLog.txt

SET errorlevel = SwithMail.exe /s /name "PNBSolver" /server "smtp.gmail.com" /p "587" /SSL /sub "PNBTest run finished" /b /Param1 "%test_name%" /from "%from%" /Username "%username%" /pass "%password%" /to "%to%" /l "%log_file%"

IF %errorlevel% ==0 (ECHO Email sent to %to%) ELSE (ECHO Error while sending email. Check %log_file%)

endlocal

