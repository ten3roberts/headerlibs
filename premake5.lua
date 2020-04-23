workspace "singlelibs"
	configurations { "debug", "release" }


newoption {
	trigger = "notest",
	description = "Autoruns the test after successful build"
}

project "test"
	kind "ConsoleApp"
	language "C"
	targetdir "bin"

	files { 
		"test.c",
		"hashtable.h",
		"mempool.h"
	}

	filter "configurations:debug"
		symbols "on"
		optimize "off"
		
	filter "configurations:release"
		symbols "off"
		optimize "on"
		
	-- Run test
	configuration "not notest"
		postbuildcommands "./bin/test"

