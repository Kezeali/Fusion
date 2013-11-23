robocopy "..\..\FusionDependencies\RakNet\Lib" "..\debug" RakNetDLLd.dll RakNetDLLd.pdb
robocopy "..\..\FusionDependencies\RakNet\Lib" "..\release" RakNetDLL.dll RakNetDLL.pdb

robocopy "..\..\FusionDependencies\libRocket\bin" "..\debug" *_d.dll *_d.pdb
robocopy "..\..\FusionDependencies\libRocket\bin" "..\release" *.dll *.pdb /XF *_d.dll *_d.pdb

robocopy "..\..\FusionDependencies\tbb_oss\bin\ia32\vc11" "..\debug" tbb_debug.* tbbmalloc_debug.*
robocopy "..\..\FusionDependencies\tbb_oss\bin\ia32\vc11" "..\release" tbb.* tbbmalloc.*

robocopy "..\..\FusionDependencies\angelscriptDev\sdk\angelscript\lib" "..\debug" *d.dll *d.pdb
robocopy "..\..\FusionDependencies\angelscriptDev\sdk\angelscript\lib" "..\release" *.dll /XF *d.dll

robocopy "..\..\FusionDependencies\bin" "..\debug" physfs.dll *d.dll *d.pdb
robocopy "..\..\FusionDependencies\bin" "..\release" *.dll /XF *d.dll

robocopy "..\..\FusionDependencies\GWEN\gwen\lib\windows\vs2010" "..\debug" gwend.dll gwend.pdb
robocopy "..\..\FusionDependencies\GWEN\gwen\lib\windows\vs2010" "..\release" gwen.dll

robocopy "..\..\FusionDependencies\Visual Leak Detector\bin\Win32" "..\debug" *