if not exist dist mkdir dist
set BuildArcId=%1
if "%1"=="" set BuildArcId="custombuild"
if exist "dist\Fusion-%BuildArcId%.7z" del "dist\Fusion-%BuildArcId%.7z"
"C:\Program Files\7-Zip\7z.exe" a "dist\Fusion-%BuildArcId%.7z" .\release_package\*