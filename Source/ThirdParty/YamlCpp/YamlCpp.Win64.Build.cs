// Fill in Copyright info...

using UnrealBuildTool;

public partial class YamlCpp : ModuleRules
{
	partial void SetupLibrary_Win64(ReadOnlyTargetRules target)
	{
		// This is generated code, not intended to be modified
		/*
			Using yaml-cpp latest 
			

			Current options:
				debug: false
				pic: true
				runtimes: MD
				shared: false
				default_features: true

			Library options help:
				registries: set the registries in vcpkg-configuration.json
				default_features: enables or disables any defaults provided by the dependency. (default: true)
				features: set the features of dependency.
				default_registries: set the default registries in vcpkg-configuration.json
				baseline: set the builtin baseline.
		*/
		PublicAdditionalLibraries.Add($"{ModuleDirectory}/LibraryFiles/yaml-cpp/Libs/Win64/{LibraryConfig}/yaml-cpp.lib");
		
		PlatformSetup = true;
	}
}