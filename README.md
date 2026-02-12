Ver1.0:

Currently, the push/pull interaction with objects has been implemented. And normally, the push/pull interaction uses a raycast emitted from the front of the player. Target pre-selection (shown as a yellow line) and a lock-on function (press F) are also in place. Once locked, you can push or pull the specified object, but the lock will be cancelled automatically if it goes out of range.

Files starting with "Level" are level-related files. "Level_00.cpp" is the simple test scene I've set up for now. More levels can be created later by continuing the numbering. The active level is selected in "MyGame.cpp" inside "InitWorld()" there are comments there so it should be easy to find. Feel free to modify that part to run your own levels. Besides, I think we'll eventually need a system to connect multiple completed levels together, but that can be addressed later.

In "Level.h" and "Level.cpp", there are functions for spawning cubes, floors and player into the world. If anyone wants to add new objects, models, or experimental gameplay elements, you can modify those files to add them.

Finally, the OBB collision issue is still not fully resolved. The current implementation uses AABB collision. If anyone has a stable OBB collision solution, feel free to improve or replace it.

Ver1.1 (Feb 12):

Added a dialogue system and a middleware folder(With a json.hpp. From now on, we can use JSON to store data).

The rendering pipeline has been updated to use UBO/SSBO (the transition is not fully complete yet). Please note that this change will not bring any visual improvement to rendering. The current version runs stably.
