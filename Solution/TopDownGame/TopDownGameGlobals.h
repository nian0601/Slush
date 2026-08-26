#pragma once

class Level;

class TopDownGameGlobals
{
public:
	static TopDownGameGlobals& GetInstance();
	static void Destroy();

	void SetLevel(Level* aLevel) { myLevel = aLevel; }
	Level& GetLevel();

private:
	TopDownGameGlobals() = default;
	~TopDownGameGlobals() = default;

	static TopDownGameGlobals* ourInstance;

	Level* myLevel = nullptr;
};
