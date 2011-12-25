robocopy "..\..\FusionDependencies\RakNet_PC-4.033\Lib" "..\debug" RakNetDebug.dll RakNetDebug.pdb
robocopy "..\..\FusionDependencies\RakNet_PC-4.033\Lib" "..\release" RakNet.dll RakNet.pdb

robocopy "..\..\FusionDependencies\libRocket\bin" "..\debug" *_d.dll *_d.pdb
robocopy "..\..\FusionDependencies\libRocket\bin" "..\release" *.dll *.pdb /XF *_d.dll *_d.pdb

robocopy "..\..\FusionDependencies\tbb30_174oss\bin\ia32\vc10" "..\debug" tbb_debug.* tbbmalloc_debug.*
robocopy "..\..\FusionDependencies\tbb30_174oss\bin\ia32\vc10" "..\release" tbb.* tbbmalloc.*

robocopy "..\..\FusionDependencies\bin" "..\debug" physfs.dll *d.dll *d.pdb
robocopy "..\..\FusionDependencies\bin" "..\release" *.dll

robocopy "..\..\RAS\library" "..\debug" *_d.dll *_d.pdb
robocopy "..\..\RAS\library" "..\release" *.dll /XF *_d.dll