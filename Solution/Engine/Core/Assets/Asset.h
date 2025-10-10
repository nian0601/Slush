#pragma once

namespace Slush
{
	class Asset
	{
	public:
		Asset(const char* aName);
		virtual ~Asset() {}
		virtual void Load(const char* aFilePath);
		virtual void Save();
		virtual void BuildUI() {};

		virtual const char* GetTypeName() = 0;
		virtual const char* GetTypeExtention() = 0;
		virtual const char* GetTypeFolder() = 0;

		const FW_String& GetAssetName() const { return myAssetName; }
		const FW_String& GetFilePath() const { return myFilePath; }
		const FW_String& GetAbsoluteFilePath() const { return myAbsoluteFilePath; }

	protected:
		FW_String myAssetName;
		FW_String myFilePath;
		FW_String myAbsoluteFilePath;
	};

#define DEFINE_ASSET(AssetName, AssetExtention, AssetFolder)\
	static const char* GetAssetTypeName() { return AssetName; }\
	static const char* GetAssetTypeExtention() { return AssetExtention; }\
	static const char* GetAssetTypeFolder() { return AssetFolder; }\
	const char* GetTypeName() override { return GetAssetTypeName(); }\
	const char* GetTypeExtention() override { return GetAssetTypeExtention(); }\
	const char* GetTypeFolder() override { return GetAssetTypeFolder(); }\

}
