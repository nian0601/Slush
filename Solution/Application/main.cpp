#include "Engine.h"

int main()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	engine.Run();
	engine.Shutdown();
	
	return 0;
}