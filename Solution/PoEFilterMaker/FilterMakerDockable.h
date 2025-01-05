#pragma once

#include "Core/Dockables/Dockable.h"
#include <FW_FileProcessor.h>

struct FilterElement
{
	void BuildUI();
	void Process(FW_FileProcessor& aProcessor);

	virtual void OnBuildUI() = 0;
	virtual void OnProcess(FW_FileProcessor& aProcessor) = 0;

	bool myEnable = true;
};

struct MinimapIcon : public FilterElement
{
	static constexpr const char* IconSizes[3] = 
	{
		"Small",
		"Medium",
		"Big",
	};

	static constexpr const char* IconColors[11] =
	{
		"Red",
		"Green",
		"Blue",
		"Brown",
		"White",
		"Yellow",
		"Cyan",
		"Grey",
		"Orange",
		"Pink",
		"Purple"
	};

	static constexpr const char* IconShapes[12] =
	{
		"Circle", 
		"Diamond", 
		"Hexagon", 
		"Square", 
		"Star", 
		"Triangle", 
		"Cross", 
		"Moon", 
		"Raindrop", 
		"Kite", 
		"Pentagon", 
		"UpsideDownHouse"
	};

	int myIconSizeIndex = 1;
	int myIconColorIndex = 2;
	int myIconShapeIndex = 2;

	void OnBuildUI() override;
	void OnProcess(FW_FileProcessor& aProcessor) override;
};

struct FilterColor : public FilterElement
{
	void OnBuildUI() override;
	void OnProcess(FW_FileProcessor& aProcessor) override;

	float myColor[4];
	const char* myColorType;
};

struct TextColor : public FilterColor
{
	TextColor()
	{
		myColor[0] = 255.f / 255.f;
		myColor[1] = 207.f / 255.f;
		myColor[2] = 132.f / 255.f;
		myColor[3] = 1.f;

		myColorType = "TextColor";
	}
};

struct BorderColor : public FilterColor
{
	BorderColor()
	{
		myColor[0] = 255.f / 255.f;
		myColor[1] = 207.f / 255.f;
		myColor[2] = 132.f / 255.f;
		myColor[3] = 1.f;

		myColorType = "BorderColor";
	}
};

struct BackgroundColor : public FilterColor
{
	BackgroundColor()
	{
		myColor[0] = 76.f / 255.f;
		myColor[1] = 51.f / 255.f;
		myColor[2] = 12.f / 255.f;
		myColor[3] = 1.f;

		myColorType = "BackgroundColor";
	}
};

struct FilterBlock
{
	FilterBlock()
	{
		myTitle = "New Block";
		myShow = true;

		myFontSize = 30;
	}

	FW_String myTitle;
	bool myShow;

	TextColor myTextColor;
	BorderColor myBorderColor;
	BackgroundColor myBackgroundColor;
	int myFontSize;

	MinimapIcon myMinimapIcon;

	void BuildUI();
	void Process(FW_FileProcessor& aProcessor);
};


class FilterMakerDockable : public Slush::Dockable
{
public:
	FilterMakerDockable();
	const char* GetName() const override { return "Filter Maker"; }

protected:
	void OnBuildUI() override;

private:
	FilterBlock myBlock;
};