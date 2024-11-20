### PLUGINS FOR RECORDING VIDEO FROM AMBF CAMERAS
This repo has two plugins, an **object plugin**, and a **simulator plugin** that use **FFMPEG** to copy the viewport contents to an .MP4. The **object plugin** is attached to a camera in the `ADF/camera.yaml` file and the **simulator plugin** can launched directly with command line arguments. The MP4 file is saved in the directory from where `ambf_simulator` is launched from.

## Compile and Build
```bash
cd <path to this directory/>
mkdir build && cd build
cmake ..
make
```

The make command will produce two library (.so) file, one is an object plugin and the other is a simulator plugin.

## Run
### a. Run the Object Plugin
Make sure `ambf_simulator` is aliased, or head to the directory where `ambf_simulator` is located. On Linux, this is `ambf/bin/lin-x86_64/`. Replace the `<path to this directory>` in the commands below with the **absolute** or **relative** location of this directory.

```bash
ambf_simulator -a <path to this directory>/ADF/camera.yaml -l1
```

### b. Run the simulator plugin
```bash
ambf_simulator --plugins <path to this directory>/build/libworld_video_recording.so
```
