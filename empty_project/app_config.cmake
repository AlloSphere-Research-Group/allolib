# list your app files here
set(app_files_list
	main.cpp
)

# other directories to include?
# ${al_homepath} holds user's home path
set(app_include_dirs
)

# where is allolib?
# this doesn't get used if app is run via run script in Linux or MacOS
# ${app_path} holds path to current project
set(user_al_path ${app_path}/../allolib)