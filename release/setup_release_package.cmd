robocopy ..\release ..\release_package\ /MIR /MT /XD .svn AS_DEBUG /XF setup_release_package.cmd *.ilk *.exp *.lib *.pdb
robocopy ..\bin ..\release_package\ /E /MT /XD .svn AS_DEBUG /XF setup.cmd *.pdb angelscriptd.dll *Debug.dll *_debug.dll *_d.dll 
exit 0
