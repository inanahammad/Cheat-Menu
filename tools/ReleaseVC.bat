@echo off
echo --------------------------------------------------
echo Building ReleaseVC
echo --------------------------------------------------
echo[
call "tools\Setup.bat"
MsBuild CheatMenu.sln /property:Configuration=Release /t:CheatMenuVC
del %SA_DIR%"\CheatMenuVC.asi" /Q
%systemroot%\System32\xcopy /s "bin\CheatMenuVC.asi" %VC_DIR% /K /D /H /Y 
