#pragma once

#include "Asset.h"
#include "AssetParser.h"

namespace Slush
{
	class DataAsset : public Asset
	{
	public:
		using Asset::Asset;
		virtual ~DataAsset() {}
		void Load(const char* aFilePath) final;
		void Save() final;

	protected:
		virtual void OnParse(AssetParser::Handle /*aRootHandle*/) {};
	};
}
