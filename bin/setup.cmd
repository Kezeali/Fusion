robocopy "..\..\FusionDependencies\RakNet_PC-4.033\Lib" "." *.dll *.pdb
robocopy "..\..\FusionDependencies\libRocket\bin" "." *.dll *.pdb
robocopy "..\..\FusionDependencies\tbb30_174oss\bin\ia32\vc10" "." tbb.* tbbmalloc.* tbb_debug.* tbbmalloc_debug.*
robocopy "..\..\FusionDependencies\bin" "." *.dll *.pdb
robocopy "..\..\RAS\library" "." *.dll *.pdb