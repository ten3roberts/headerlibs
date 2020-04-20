workspace "singlelibs"
	configurations { "debug", "release" }

project "test"
	kind "ConsoleApp"
	language "C"
	targetdir "bin"

	files { 
		"test.c",
		"hashtable.h"
	}

	filter "configurations:debug"
		symbols "on"
		optimize "off"

	filter "configurations:release"
		symbols "off"
		optimize "on"

