// Fill in Copyright info...

using UnrealBuildTool;

public partial class RangeV3 : ModuleRules
{
	partial void SetupLibrary_Includes(ReadOnlyTargetRules target)
	{
		// This is generated code, not intended to be modified
		/*
			Using range-v3 0.12.0 
			Range library for C++14/17/20, basis for C++20's std::ranges

			Current options:
				runtimes: MD
				shared: false
				pic: true
				debug: false

			Library options help:
				
		*/
		PublicSystemIncludePaths.Add($"{ModuleDirectory}/LibraryFiles/range-v3/SysIncludes/include");
		
		PlatformSetup = true;
		IncludesSetup = true;
	}
}