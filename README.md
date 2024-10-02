This is an attempt to implement the Learning Agents basic tutorial on the [Unreal Engine site](https://dev.epicgames.com/community/learning/courses/M3D/unreal-engine-learning-agents-getting-started/8OWY/unreal-engine-learning-agents-introduction) but in C++ instead of Blueprint.

**UPDATE**: This has been re-written for UE 5.4.4 where the learning agents plugin has significant changes. The base tutorial was rewritten so that has been followed and then much of it moved to C++ as with the previous version.

## Basic Function
All functionality from the tutorial has been implemented in C++ along with a few additional features.

## New Features
There's a few features I've added beyond the original tutorial:
- The Spline Component needed to follow the track is built automatically from the landscape spline actor.

- The Manager can be easily switch between reinitializing the neural network, continued learning, and inference.
  - There is a enum variable exposed on the manager to select the mode.

- Agents can learn to drive in traffic (with varying degrees of success)
  - Agents can observe the position and velocity of the closest other agents around them.
  - This allows you to leave collisions enabled so they can learn to avoid each other.
  - You can set how many agents to observe in order of distance from the agent.
  - They are penalized for collisions with other agents and eventually reset if they collide too many times.

- Agents can learn to drive stick (WIP)
  - Observations have been added for the engine RPMs and the gear the agent is in.
  - The agent is rewarded for staying in the RPM sweet spot.
  - An action is added to the interactor for shifting up or down or staying in the same gear.
  - This has not been tested yet to see if the reward is enough to learn to shift gears effectively.
