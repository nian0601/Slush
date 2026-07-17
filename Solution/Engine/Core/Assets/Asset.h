#pragma once
#include <FW_TypeID.h>

namespace Slush
{
	class Asset
	{
	public:
		Asset(const char* aName, unsigned int aAssetID);
		virtual ~Asset() {}
		virtual void Load(const char* aFilePath);
		virtual void Save();
		virtual void BuildUI() {};
		virtual void ResolveDependencies() {};

		virtual const char* GetTypeName() const = 0;
		virtual const char* GetTypeExtention() const = 0;
		virtual const char* GetTypeFolder() const = 0;
		virtual unsigned int GetCurrentAssetVersion() const = 0;

		const FW_String& GetAssetName() const { return myAssetName; }
		const FW_String& GetFilePath() const { return myFilePath; }
		const FW_String& GetAbsoluteFilePath() const { return myAbsoluteFilePath; }
		unsigned int GetAssetTypeID() const { return myAssetTypeID; }

	protected:
		FW_String myAssetName;
		FW_String myFilePath;
		FW_String myAbsoluteFilePath;
		unsigned int myAssetTypeID = INT_MAX;
	};

#define DEFINE_ASSET(AssetName, AssetExtention, AssetFolder, Version)\
	static const char* GetAssetTypeName() { return AssetName; }\
	static const char* GetAssetTypeExtention() { return AssetExtention; }\
	static const char* GetAssetTypeFolder() { return AssetFolder; }\
	const char* GetTypeName() const override { return GetAssetTypeName(); }\
	const char* GetTypeExtention() const override { return GetAssetTypeExtention(); }\
	const char* GetTypeFolder() const override { return GetAssetTypeFolder(); }\
	unsigned int GetCurrentAssetVersion() const override { return Version; }\


	template <typename AssetType>
	unsigned int GetAssetID()
	{
		return FW_TypeID<Asset>::GetID<AssetType>();
	}
}
