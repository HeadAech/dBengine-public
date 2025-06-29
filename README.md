# dBengine public
This is the public repository containing <b>dBengine</b> and its' editor. Compile the project using provided CMakeLists file.

## Learn more 
To learn more about the engine, visit [author's blog](headeach.github.io).

# How to run dBeditor? <br>
To run dBeditor all you need to do is launch the dBengine.exe. That's all.

# How to run a scene in runtime?
To run a scene in runtime, you can use command line arguments.

To run dBengine from a scene, you need to enter: `.\dBengine.exe --scene "path to scene"`, where path is placed in res directory for user's convinience.

To run dBengine from a scene with editor disabled, you need to add another argument: `--no-editor`.
So final command would look like: `.\dBengine.exe --no-editor --scene "path to scene"`.
