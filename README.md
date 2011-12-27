**[Fusion Project Page][1]** 

Build instructions for Windows with VC2010:
===========================================
1.  Create a "workspace" folder in which all the following folders will be created
2.  Get **Fusion**, **[RAS][2]** and **[ScriptUtils][3]**
3.  Install the dependencies: 
    1.  Create a new folder called **FusionDependencies**
    2.  Download and extract [Fusion Dependencies][4] (headers and binaries compiled for VC2010) into the folder you just created 
4.  Open and build *ScriptUtils\ScriptUtils.sln*
5.  Open and build *RAS\libRocket_AngelScript.sln*
6.  You can now open and build *Fusion\Fusion.sln*
7.  Done!

Oops, one last step: Run ***bin/setup_targetdirs.cmd*** before debugging Fusion within Visual Studio: this should copy all the .dlls from *FusionDependencies\bin*, *FusionDependencies\libRocket\bin*, *FusionDependencies\RakNet-x.x\Lib*, *RAS\library*, etc. into *Fusion\$(ConfigName)*, the working directory used by the executable projects. 

Pre-compiled builds:
====================
Go [here][4] to download up-to-date builds.

 [1]: http://sourceforge.net/projects/steelfusion
 [2]: https://github.com/Kezeali/RAS
 [3]: https://github.com/Kezeali/ScriptUtils
 [4]: http://files.elliothayward.net/FusionDependencies.7z
 [5]: http://files.elliothayward.net/releases