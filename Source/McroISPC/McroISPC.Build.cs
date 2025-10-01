/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *  
 *  @author David Mórász
 *  @date 2025
 */

using UnrealBuildTool;
using McroBuild;

/// <summary>
/// A module containing ISPC utilities.
/// </summary>
public class McroISPC : ModuleRules
{
	public McroISPC(ReadOnlyTargetRules Target) : base(Target)
	{
		bUseUnity = false;
		
		PublicDependencyModuleNames.AddRange(new[] {
			"Core",
		});
			
		
		PrivateDependencyModuleNames.AddRange(new[] {
			"CoreUObject",
		});
	}
}
