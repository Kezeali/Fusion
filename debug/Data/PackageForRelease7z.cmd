rem The path to 7-zip.exe below works for 32-bit OSes with 32-bit 7-zip and 64-bit OSes with 64-bit 7-zip, so I think that covers most setups
mkdir ..\..\release\Data
del ..\..\release\Data\core.7z ..\..\release\Data\game.7z
"C:\Program Files\7-Zip\7z.exe" a ..\..\release\Data\core.7z @package_core.txt -m"s=off"
"C:\Program Files\7-Zip\7z.exe" a ..\..\release\Data\game.7z @package_game.txt -m"s=off"
