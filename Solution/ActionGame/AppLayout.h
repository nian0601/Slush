#pragma once

class AppLayout
{
public:
	virtual ~AppLayout() {};
	virtual void Update() = 0;
	virtual void Render() = 0;
};
