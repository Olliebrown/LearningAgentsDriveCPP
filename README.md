This is an attempt to implement the Learning Agents basic tutorial on the [Unreal Engine site](https://dev.epicgames.com/community/learning/courses/M3D/unreal-engine-learning-agents-getting-started/8OWY/unreal-engine-learning-agents-introduction) but in C++ instead of Blueprint.

WARNING: This is WORK IN PROGRESS!

Right now there are some bugs that I haven't be able to work around:
- When the learning Manager is a pure C++ class, I cannot increase the max agents above 1.
- When the learning Manager is wrapped with a Blueprint class, the components do not properly initialize.

I have asked questions about these issues in the UE forums. If you have any ideas, please let me know!
- [Question specific to Learning Agents](https://forums.unrealengine.com/t/learningagentmanager-in-pure-c-setting-maxagents-or-components-cant-find-manager/1310438)
- [More general question about UObject construction time parameters](https://forums.unrealengine.com/t/providing-construction-time-critical-state-to-a-parent-from-the-child/1310455)

## New Features
There's a few features I've added beyond the original tutorial:
- I have made a special vehicle pawn specifically for autonomous control.
  - This new pawn has no cameras or input components as they are not needed.
  - It also has a special arrow to visualize the controller input coming from the learning agent.
- There are autonomous pawns for BOTH the sports car and the off-road car.
- The Spline Component needed to follow the track is built automatically from the landscape spline actor.
  - Code for this is inside the LearningManager c++ class.
