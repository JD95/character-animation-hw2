# Character Animation HW2

## Contribute

Please develop on either the `collect-sync-frame` or `time-warp` branches. When a feature is complete we can merge them with dev, leaving merges with master to be done only when we know things are absolutely working.

## Collect Sync Frames 
- Collect distance of each foot traveled each frame in seperate arrays
- Take first derivative of both arrays to get foot velocities
- Search each array for when velocity hits zero, these are the steps

## Time Warp
- Use the step information to create ratios for each of the character's step rate
- Sync the times
