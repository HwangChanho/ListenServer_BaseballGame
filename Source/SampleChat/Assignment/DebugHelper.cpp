#include "DebugHelper.h"

void DebugHelper::PrintDebugMessage(const FString& Message, float DisplayTime, FColor Color)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DisplayTime, Color, Message);
	}
}