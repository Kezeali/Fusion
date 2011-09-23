robocopy ..\debug ..\multiplayer_debug\ /MIR /MT /XD .svn AS_DEBUG /XF setup_multiplayer_debug.cmd
robocopy ..\bin ..\multiplayer_debug\ /E /MT /XD .svn AS_DEBUG
call host_mp.cmd
exit 0
