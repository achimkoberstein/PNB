@ECHO OFF
REM We cd to the PNB project folder so that all relative paths in NestedBendersTest.cpp file are correct.
cd ..\ParallelNestedBenders\MSVisualStudio\v16alt\NestedBendersTest

REM Copy cplex .dll file
xcopy "..\..\..\..\ThirdParty\cplex\cplex1290\lib\x64\cplex1290.dll" "..\..\..\..\x64\Release\bin\" /Y

REM Choose test
..\..\..\..\x64\Release\bin\NestedBendersTest.exe --gtest_list_tests
ECHO Choose test(s) to run.
ECHO Provide test_set.test_case, e.g., NestedBendersTest.fast
ECHO or use wildcard to run several tests, e.g., *.fast

SET /p test="Test to run: "

REM Copy cplex .dll file
xcopy "..\..\..\..\ThirdParty\cplex\cplex1290\lib\x64\cplex1290.dll" "..\..\..\..\x64\Release\bin\" /Y

REM Start test
..\..\..\..\x64\Release\bin\NestedBendersTest.exe --gtest_filter="%test%"

timeout 10

REM Send email upon test end
..\..\..\..\scripts\SwithMail\PNBTest_finished_email.bat PNBTest yes

timeout 10