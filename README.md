This is an attempt to implement the Learning Agents basic tutorial on the [Unreal Engine site](https://dev.epicgames.com/community/learning/courses/M3D/unreal-engine-learning-agents-getting-started/8OWY/unreal-engine-learning-agents-introduction) but in C++ instead of Blueprint.

## Basic Function
All critical bugs have been addressed and it now functions at least as well as the original tutorial.  I have also added a few new quality-of-life features.

## New Features
There's a few features I've added beyond the original tutorial:
- I have made a special vehicle pawn specifically for autonomous control.
  - This new pawn has no cameras or input components as they are not needed.
  - It also has a special arrow to visualize the controller input coming from the learning agent.
- There are autonomous pawns for BOTH the sports car and the off-road car.
- The Spline Component needed to follow the track is built automatically from the landscape spline actor.
  - Code for this is inside the LearningManager c++ class.
- The Manager can be toggled into an inference mode instead of a learning mode.
- There is a boolean variable exposed on the manager to toggle resetting the neural network.
