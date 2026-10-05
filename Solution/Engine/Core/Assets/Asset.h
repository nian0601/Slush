#pragma once
#include <FW_TypeID.h>
#include "imgui/Fonts/IconsFontAwesome7.h"

namespace Slush
{
	class Asset
	{
	public:
		Asset(const char* aName, unsigned int aAssetID);
		virtual ~Asset() {}
		// aFilePath is relative to the Asset Repository at aRepositoryIndex
		virtual void Load(const char* aFilePath, int aRepositoryIndex);
		virtual void Save();
		virtual void BuildUI() {};
		virtual void ResolveDependencies() {};

		virtual const char* GetTypeName() const = 0;
		virtual const char* GetTypeExtention() const = 0;
		virtual const char* GetTypeFolder() const = 0;
		virtual const char* GetTypeIcon() const = 0;
		virtual unsigned int GetCurrentAssetVersion() const = 0;

		const FW_String& GetAssetName() const { return myAssetName; }
		const FW_String& GetFilePath() const { return myFilePath; }
		int GetRepositoryIndex() const { return myRepositoryIndex; }
		const FW_String& GetRepositoryName() const;

		// Where the next Save() writes. New assets and copies go to the first mount (index 0).
		void SetRepositoryIndex(int aRepositoryIndex) { myRepositoryIndex = aRepositoryIndex; }
		unsigned int GetAssetTypeID() const { return myAssetTypeID; }

		bool HasUnsavedChanges() const { return myHasUnsavedChanges; }
		void MarkAsUnsaved() { myHasUnsavedChanges = true; }
		void MarkAsSaved() { myHasUnsavedChanges = false; }

	protected:
		FW_String myAssetName;
		FW_String myFilePath;
		FW_String myAbsoluteFilePath;
		int myRepositoryIndex = 0;
		unsigned int myAssetTypeID = INT_MAX;
		bool myHasUnsavedChanges = false;
	};

#define DEFINE_ASSET(AssetName, AssetExtention, AssetFolder, AssetIcon, Version)\
	static const char* GetAssetTypeName() { return AssetName; }\
	static const char* GetAssetTypeExtention() { return AssetExtention; }\
	static const char* GetAssetTypeFolder() { return AssetFolder; }\
	static const char* GetAssetTypeIcon() { return AssetIcon; }\
	const char* GetTypeName() const override { return GetAssetTypeName(); }\
	const char* GetTypeExtention() const override { return GetAssetTypeExtention(); }\
	const char* GetTypeFolder() const override { return GetAssetTypeFolder(); }\
	const char* GetTypeIcon() const override { return GetAssetTypeIcon(); }\
	unsigned int GetCurrentAssetVersion() const override { return Version; }\


	template <typename AssetType>
	unsigned int GetAssetID()
	{
		return FW_TypeID<Asset>::GetID<AssetType>();
	}
}
