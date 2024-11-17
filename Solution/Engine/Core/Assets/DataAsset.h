#pragma once

#include "Asset.h"
#include "AssetParser.h"

namespace Slush
{
	class DataAsset : public Asset
	{
	public:
		DataAsset(const char* aName);
		virtual ~DataAsset() {}
		void Load(const char* aFilePath) final;
		void Save() final;

	protected:
		virtual void OnParse(AssetParser::Handle /*aRootHandle*/) {};
	};
}
