# WINDOWS is set for Windows, MACOS for MacOS, LINUX for Linux
# ${home_path} holds user's home path
# ${app_path} holds path to current project

# list your app files here
set(app_files_list
	main.cpp
)

# other directories to include?
set(app_include_dirs
)

# where is allolib?
# this doesn't get used if app is run via run script in Linux or MacOS
set(user_al_path ${app_path}/../allolib)