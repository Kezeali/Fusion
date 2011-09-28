robocopy ..\debug ..\multiplayer_debug\ /MIR /MT /XD .svn AS_DEBUG /XF setup_multiplayer_debug.cmd *.ilk
robocopy ..\bin ..\multiplayer_debug\ /E /MT /XD .svn AS_DEBUG
exit 0
