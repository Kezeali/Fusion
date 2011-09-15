robocopy ..\debug ..\multiplayer_debug\ /MIR /XD .svn AS_DEBUG /XF setup_multiplayer_debug.cmd
copy ..\bin\* ..\multiplayer_debug\
call host_mp.cmd
exit 0
