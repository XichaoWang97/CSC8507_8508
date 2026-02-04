Ver0.1 :

Objects with different colors: Blue (North polarity), Orange (South polarity)

Anchor: Set InverseMass to 0.0. The anchor cube will not move, and you can pull yourself toward the anchor’s position.

Player: Click LMB/RMB to temporarily change polarity.

Besides, you can see a rotating cube, which uses an OBB collider. This part is not stable, and I still haven’t managed to fix it so far. 
If anyone has a way to fix the OBB collision, please feel free to jump in and take a look!

The levels are built in MyGame.cpp, and the player mechanics are implemented in the Player class.

Ver0.12(Branch NoPolarity):

The magnetic system has been removed and replaced with left-click/right-click push and pull mechanics. The interaction forces between the player and objects have been further refined. The system is currently undergoing parameter tuning, and the target selection feature is still under development.
