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

		virtual const char* GetTypeName() const = 0;
		virtual const char* GetTypeExtention() const = 0;
		virtual const char* GetTypeFolder() const = 0;

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

#define DEFINE_ASSET(AssetName, AssetExtention, AssetFolder)\
	static const char* GetAssetTypeName() { return AssetName; }\
	static const char* GetAssetTypeExtention() { return AssetExtention; }\
	static const char* GetAssetTypeFolder() { return AssetFolder; }\
	const char* GetTypeName() const override { return GetAssetTypeName(); }\
	const char* GetTypeExtention() const override { return GetAssetTypeExtention(); }\
	const char* GetTypeFolder() const override { return GetAssetTypeFolder(); }\


	template <typename AssetType>
	unsigned int GetAssetID()
	{
		return FW_TypeID<Asset>::GetID<AssetType>();
	}
}
