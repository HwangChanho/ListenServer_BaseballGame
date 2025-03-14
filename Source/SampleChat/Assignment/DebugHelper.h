#pragma once

#include "CoreMinimal.h"

class SAMPLECHAT_API DebugHelper
{
public:
	static void PrintDebugMessage(const FString& Message, float DisplayTime = 10.0f, FColor Color = FColor::Emerald);
};
