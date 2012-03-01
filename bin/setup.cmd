rem This is the old setup script that copies the lib binaries into bin
rem You should use setup_targetdirs.cmd instead
robocopy "..\..\FusionDependencies\RakNet_PC-4.033\Lib" "." *.dll *.pdb /XF *staticdebug.pdb
robocopy "..\..\FusionDependencies\libRocket\bin" "." *.dll *.pdb
robocopy "..\..\FusionDependencies\tbb_oss\bin\ia32\vc10" "." tbb.* tbbmalloc.* tbb_debug.* tbbmalloc_debug.*
robocopy "..\..\FusionDependencies\bin" "." *.dll *.pdb
robocopy "..\..\RAS\library" "." *.dll *.pdb