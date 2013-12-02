Build instructions for Windows with VC2012:
===========================================
1.  Create a "workspace" folder in which all the following folders will be created
2.  Get **Fusion** and **[ScriptUtils][2]**
3.  Install the dependencies: 
    1.  Create a new folder called **FusionDependencies**
    2.  Download and extract [FusionDependencies.7z][3] (headers and binaries compiled for VC2012) into the folder you just created 
5.  Open and build *Fusion\Fusion.sln*
6.	Run ***bin/setup_targetdirs.cmd*** before debugging Fusion within Visual Studio: this should copy all the .dlls from the dependencies into each of the working directories used by the executable projects. 
7.  Done!

Release builds:
====================
Go [here][4] to download up-to-date builds.

**Old [Fusion Project Page][1] on sourceforge** 

 [1]: http://sourceforge.net/projects/steelfusion
 [2]: https://github.com/Kezeali/ScriptUtils
 [3]: http://files.elliothayward.net/FusionDependencies.7z
 [4]: http://files.elliothayward.net/releases