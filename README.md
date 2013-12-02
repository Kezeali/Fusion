Building on Windows with VC2012
===========================================
1.  Get **Fusion** and make sure to get all the submodules.
3.  Install the dependencies: 
    1.  Create a new folder called **FusionDependencies** in the Fusion base folder
    2.  Download and extract [FusionDependencies.7z][3] (headers and binaries compiled for VC2012) into this new folder
5.  Open and build *Fusion\Fusion.sln*
6.	Run ***bin/setup_targetdirs.cmd*** before debugging Fusion within Visual Studio: this should copy all the .dlls from the dependencies into each of the working directories used by the executable projects. 
7.  Done!

Release builds
=================
Go [here][4] to download up-to-date builds.

Stuff
=======
Old [Fusion Project Page][1] on sourceforge

 [1]: http://sourceforge.net/projects/steelfusion
 [2]: https://github.com/Kezeali/ScriptUtils
 [3]: http://files.elliothayward.net/FusionDependencies.7z
 [4]: http://files.elliothayward.net/releases