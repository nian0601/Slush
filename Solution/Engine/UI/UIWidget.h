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
		virtual void Inactivate() {};

		FW_String myIdentifier;
	};
}
