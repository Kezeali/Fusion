rem The path to 7-zip.exe below works for 32-bit OSes with 32-bit 7-zip and 64-bit OSes with 64-bit 7-zip, so I think that covers most setups
mkdir ..\..\release\Data
del ..\..\release\Data\core.zip ..\..\release\Data\game.zip
"C:\Program Files\7-Zip\7z.exe" a ..\..\release\Data\core.zip @package_core.txt -xr!*.svn
"C:\Program Files\7-Zip\7z.exe" a ..\..\release\Data\game.zip @package_game.txt -xr!*.svn
