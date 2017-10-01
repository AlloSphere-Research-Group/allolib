MacOS & Ubuntu

1. dependencies
    - cmake
    - glfw
    - glew
    - pkg-config
    - for MacOS using brew is recommended

2. compiling the lib
    - in `al_lib` directory,
    - `git submodule init`
    - `git submodule update` (clones in source dependency files)
    - `mkdir build`
    - `cmake ..`
    - `make`
    - will output lib file in `al_lib` dir
    - to use, link the lib and include the dir `include`

3. preparing a project
    - project folder structure
    
    ```
    project folder
        |- CMakeLists.txt
        |- app_config.cmake (user defined app configurations)
        |- bin/ (app executable gets generated here)
            |- data/ (where app assets are put)
    ```

    - in `app_config.cmake` file, user can specify the project files list
    - example empty project can be found in al_lib repo

4. running a project
    - `mkdir build`
    - `cmake ..`
    - `make`
    - will output executable file in `build/` dir with same name as the project folder

5. run script
    - is in al_proj repo, and will do step 2 & 4 automatically
    - should be next to `al_lib/` folder
    
    ```
    al_proj/
        |- al_lib/
        |- run.sh
    ```
    
    - `./run.sh path/to/project/`
    - for both `run.sh` and `path/to/project/`, either relative or absolute path will work
        - ex) `~/frameworks/al_proj/run.sh ./apps/my_project01/`

5. other