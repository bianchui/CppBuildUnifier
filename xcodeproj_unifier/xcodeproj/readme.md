#Header

	// !$*UTF8*$!
	{
		archiveVersion = 1;
		classes = {
		};
		objectVersion = 46;
		objects = {


#PBXBuildFile

	/* Begin PBXBuildFile section */
		BUILD_GUID /* FILE_NAME in Sources */ = {isa = PBXBuildFile; fileRef = FILE_GUID /* FILE_NAME */; };
		ED4258742081CB1A00F5CDDA /* CoreValue.cpp in Sources */ = {isa = PBXBuildFile; fileRef = ED4258722081CB1A00F5CDDA /* CoreValue.cpp */; };
		ED800F6720941E47008D9316 /* Foundation.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = ED800F6620941E47008D9316 /* Foundation.framework */; };
	/* End PBXBuildFile section */


#PBXCopyFilesBuildPhase

	/* Begin PBXCopyFilesBuildPhase section */
		ED505738206A4C46009B5ADB /* CopyFiles */ = {
			isa = PBXCopyFilesBuildPhase;
			buildActionMask = 2147483647;
			dstPath = /usr/share/man/man1/;
			dstSubfolderSpec = 0;
			files = (
			);
			runOnlyForDeploymentPostprocessing = 1;
		};
	/* End PBXCopyFilesBuildPhase section */


#PBXFileReference

	/* Begin PBXFileReference section */
		FILE_GUID /* FILE_NAME */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.cpp.cpp; path = FILE_NAME; sourceTree = "<group>"; };
		ED4258722081CB1A00F5CDDA /* CoreValue.cpp */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.cpp.cpp; path = CoreValue.cpp; sourceTree = "<group>"; };
		ED800F6620941E47008D9316 /* Foundation.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Foundation.framework; path = System/Library/Frameworks/Foundation.framework; sourceTree = SDKROOT; };
	/* End PBXFileReference section */


###FileType

| ext       | lastKnownFileType |
| --------- | --- |
| h         | sourcecode.c.h |
| pch       | sourcecode.c.h |
| c         | sourcecode.c.c |
| m         | sourcecode.c.objc |
| hpp       | sourcecode.cpp.h |
| cpp       | sourcecode.cpp.cpp |
| mm        | sourcecode.cpp.objcpp |
| swift     | sourcecode.swift |
| js        | sourcecode.javascript |
| a         | archive.ar |
| dylib     | "compiled.mach-o.dylib" |
| tbd       | "sourcecode.text-based-dylib-definition" |
| framework | wrapper.framework |
| xcodeproj | "wrapper.pb-project" |
| app       | wrapper.application |
| appex     | wrapper.app-extension |
| bundle    | wrapper.plug-in |
|xcdatamodel| wrapper.xcdatamodel |
| mdimporter| wrapper.cfbundle |
| octest    | wrapper.cfbundle |
| xctest    | wrapper.cfbundle |
| xcassets  | folder.assetcatalog |
| xib       | file.xib |
| rl        | text |
| markdown  | text |
| md        | net.daringfireball.markdown |
| plist     | text.plist.xml |
| sh        | text.script.sh |
| xcconfig  | text.xcconfig |
| strings   | text.plist.string |


###sourceTree

| sourceTree | Location |
| ---------- | --- |
|"\<absolute\>"| Absolute Path |
|"\<group\>" | Relative to Group |
|SOURCE\_ROOT| Relative to Project |
|DEVELOPER\_DIR| Relative to Developer Directory |
| SDKROOT    | Relative to SDK |
|BUILT\_PRODUCTS\_DIR| Relative to Build Products |


#PBXFrameworksBuildPhase

	/* Begin PBXFrameworksBuildPhase section */
		720E670B6C491B82738970BC /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
				ED800F6720941E47008D9316 /* Foundation.framework in Frameworks */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
	/* End PBXFrameworksBuildPhase section */


#PBXGroup

	/* Begin PBXGroup section */
		36CC90619CE99619B32588BE /* Products */ = {
			isa = PBXGroup;
			children = (
				ED50573A206A4C46009B5ADB /* jscnode */,
			);
			name = Products;
			sourceTree = "<group>";
		};
		CE1625FFB3C961F8F90D7E10 = {
			isa = PBXGroup;
			children = (
				EDFA9273206C860800BFE604 /* src */,
				36CC90619CE99619B32588BE /* Products */,
				ED505744206A53A7009B5ADB /* Frameworks */,
			);
			sourceTree = "<group>";
		};
		ED50572E2069ECB6009B5ADB /* core */ = {
			isa = PBXGroup;
			children = (
				ED4258722081CB1A00F5CDDA /* CoreValue.cpp */,
			);
			path = core;
			sourceTree = "<group>";
		};
		ED505744206A53A7009B5ADB /* Frameworks */ = {
			isa = PBXGroup;
			children = (
				ED800F6620941E47008D9316 /* Foundation.framework */,
			);
			name = Frameworks;
			sourceTree = "<group>";
		};
		EDFA9273206C860800BFE604 /* src */ = {
			isa = PBXGroup;
			children = (
				ED50572E2069ECB6009B5ADB /* core */,
			);
			name = src;
			path = ../src;
			sourceTree = "<group>";
		};
	/* End PBXGroup section */


#PBXNativeTarget

	/* Begin PBXNativeTarget section */
		4B575E9EA876D25BCF5A4147 /* jscnode */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = ED505741206A4C47009B5ADB /* Build configuration list for PBXNativeTarget "jscnode" */;
			buildPhases = (
				A56C507A82F8370FD121C61B /* Sources */,
				720E670B6C491B82738970BC /* Frameworks */,
				ED505738206A4C46009B5ADB /* CopyFiles */,
			);
			buildRules = (
			);
			dependencies = (
			);
			name = jscnode;
			productName = jscnode;
			productReference = ED50573A206A4C46009B5ADB /* jscnode */;
			productType = "com.apple.product-type.tool";
		};
	/* End PBXNativeTarget section */


#PBXProject

	/* Begin PBXProject section */
		35108566DD7B7EB466D59581 /* Project object */ = {
			isa = PBXProject;
			attributes = {
				LastUpgradeCheck = 0830;
				ORGANIZATIONNAME = bianchui;
				TargetAttributes = {
					4B575E9EA876D25BCF5A4147 = {
						CreatedOnToolsVersion = 8.3.3;
						DevelopmentTeam = PEZE6XWYTJ;
						ProvisioningStyle = Automatic;
					};
				};
			};
			buildConfigurationList = ED505735206A4C46009B5ADB /* Build configuration list for PBXProject "jscnode" */;
			compatibilityVersion = "Xcode 3.2";
			developmentRegion = English;
			hasScannedForEncodings = 0;
			knownRegions = (
				en,
			);
			mainGroup = CE1625FFB3C961F8F90D7E10;
			productRefGroup = 36CC90619CE99619B32588BE /* Products */;
			projectDirPath = "";
			projectRoot = "";
			targets = (
				4B575E9EA876D25BCF5A4147 /* jscnode */,
			);
		};
	/* End PBXProject section */


#PBXSourcesBuildPhase

	/* Begin PBXSourcesBuildPhase section */
		A56C507A82F8370FD121C61B /* Sources */ = {
			isa = PBXSourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
				BUILD_GUID /* FILE_NAME in Sources */,
				ED4258742081CB1A00F5CDDA /* CoreValue.cpp in Sources */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
	/* End PBXSourcesBuildPhase section */


#XCBuildConfiguration

	/* Begin XCBuildConfiguration section */
		ED50573F206A4C47009B5ADB /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				SDKROOT = macosx;
			};
			name = Debug;
		};
		ED505740206A4C47009B5ADB /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				SDKROOT = macosx;
			};
			name = Release;
		};
		ED505742206A4C47009B5ADB /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = "$(TARGET_NAME)";
			};
			name = Debug;
		};
		ED505743206A4C47009B5ADB /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = "$(TARGET_NAME)";
			};
			name = Release;
		};
	/* End XCBuildConfiguration section */


#XCConfigurationList

	/* Begin XCConfigurationList section */
		ED505735206A4C46009B5ADB /* Build configuration list for PBXProject "jscnode" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				ED50573F206A4C47009B5ADB /* Debug */,
				ED505740206A4C47009B5ADB /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
		ED505741206A4C47009B5ADB /* Build configuration list for PBXNativeTarget "jscnode" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				ED505742206A4C47009B5ADB /* Debug */,
				ED505743206A4C47009B5ADB /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
	/* End XCConfigurationList section */


#Footer

		};
		rootObject = 35108566DD7B7EB466D59581 /* Project object */;
	}
