#include "TutorialGameMode.h"

#include "TutorialPawn.h"

ATutorialGameMode::ATutorialGameMode()
{
	DefaultPawnClass = ATutorialPawn::StaticClass();
}
