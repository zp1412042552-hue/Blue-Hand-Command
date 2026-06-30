#include "GestureRecorderGameMode.h"

#include "GestureRecorderPawn.h"

AGestureRecorderGameMode::AGestureRecorderGameMode()
{
	DefaultPawnClass = AGestureRecorderPawn::StaticClass();
}
