# MemDoc
C/C++ macro style memory leak detector using crtdbg.h library, providing memory leak reports.

Visual Studio C/C++ Memory Leak Detector : MemDoc
=================================================

Purpose
-------
Help in detecting memory leaks in C/C++ projects.


Project & System Configuration
------------------------------
Can be added to any Visual Studio project by simply adding it in your C/C++ project and including in build chain.

NOTE :: Make sure you build/run your program in Debug mode.


General How To
--------------
Add the provided .h/.cpp files in your project and build chain.
Include MemDoc.h file in your projects .h/.cpp files which are candidate of Memory leaks if known, otherwise include in all files.
It has some macros which are required in all .h/cpp file of your projects.
Add INIT_DOC at begin of your program.
Add END_DOC at end of your program.
Replace your new with DEBUG_NEW [ use Find&Replace in current project of Visual Studio ]
Make sure code block are executed where we have added INIT_DOC and END_DOC.


NMG Specific How To
-------------------
Add .h/.cpp files in your game specific library or game code if no game specific library.
Include MemDoc.h file in your projects .h/.cpp files which are candidate of Memory leaks if known, otherwise include in all file.
Add INIT_DOC in ctor of your Server.
Add END_DOC in Dispose() method of your Server [ should be in dtor but for some reason UNITY don't call dtor of our server, but GDK do call the virtual void Dispose() { that's why little ambiguity }, 
	and we should call ClearDefinitions() from Dispose for same reason ].
Replace your new with DEBUG_NEW [ use Find&Replace in current project of Visual Studio ]
Make sure code block are executed where we have added INIT_DOC and END_DOC.

NOTE : Make sure yo go to lobby before exiting the game other wise game will exit with exception, then the required methods will not get executed.


Output
------
It will create a file named "MemDoc_CrtDbgReport.txt" and "MemDoc_FilteredCrtDbgReport.txt" were your program's exe reside.
In case of NMG game it will be added to “Tools\Unity\UnityTestLobby\PreBuilt” folder.


LIMITATION
----------
Only work for memory allocated via new, if malloc/realloc is used that will not be detected.


CAUTIONS
--------
As this use some windows and visual studio's header, it may give link error if your method name collide with that macros name,
this can be simply rectified, by undefining those macros in requird game side code.