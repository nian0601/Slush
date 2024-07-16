#pragma once

namespace Slush
{
	class Input;
	class UIWidget
	{
	public:
		virtual ~UIWidget() {}

		virtual void Update(const Input& /*aInput*/) {};
		virtual void Render() {};

		FW_String myIdentifier;
	};
}
