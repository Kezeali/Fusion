if not exist ..\release\angelscript.dll call ..\bin\setup_targetdirs.cmd
robocopy ..\release ..\release_package\Fusion /MIR /MT /XD .git AS_DEBUG /XF setup_release_package.cmd *.ilk *.exp *.lib *.pdb setup.cmd
exit 0
