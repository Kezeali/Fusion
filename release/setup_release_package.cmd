robocopy ..\release ..\release_package\ /MIR /MT /XD .svn AS_DEBUG /XF setup_release_package.cmd *.ilk *exp
robocopy ..\bin ..\release_package\ /E /MT /XD .svn AS_DEBUG /XF setup.cmd
exit 0
