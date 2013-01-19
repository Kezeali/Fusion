copy filelist.txt ..\..\..\FusionDependencies\filelist.txt
cd ..\..\..\FusionDependencies
if exist FusionDependencies.7z del FusionDependencies.7z
"%ProgramFiles%\7-Zip\7z.exe" a FusionDependencies.7z @filelist.txt -mx9 -ssc -xr!*.ncb -xr!*.sdf -xr!*.user -xr!*.suo -xr!*/Debug -xr!*/Release -xr!tbb_oss/doc -xr!*/ipch -xr!*.obj -xr!*.manifest -xr!*.manifest.res -xr!kyotocabinet/*.exe