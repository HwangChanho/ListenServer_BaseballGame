#include "NumberBaseballGameState.h"


ANumberBaseballGameState::ANumberBaseballGameState()
{
	
}

void ANumberBaseballGameState::BeginPlay()
{
	Super::BeginPlay();

	
}

void ANumberBaseballGameState::MulticastBroadcastMessage_Implementation(const FString& Message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Yellow, Message);
	}
}