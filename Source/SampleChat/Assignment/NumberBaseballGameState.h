#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "NumberBaseballGameState.generated.h"

UCLASS()
class SAMPLECHAT_API ANumberBaseballGameState : public AGameState
{
	GENERATED_BODY()

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastMessage(const FString& Message);

protected:
	ANumberBaseballGameState();
	virtual void BeginPlay() override;
};
