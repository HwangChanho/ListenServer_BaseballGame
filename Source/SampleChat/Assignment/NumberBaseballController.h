#pragma once

#include "CoreMinimal.h"
#include "FNumberBaseballResult.h"
#include "GameFramework/PlayerController.h"
#include "NumberBaseballController.generated.h"

class UNumberBaseballMainWidget;

UCLASS()
class SAMPLECHAT_API ANumberBaseballController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UNumberBaseballMainWidget> BaseballClass;

	UPROPERTY()
	UNumberBaseballMainWidget* BaseballInstance;

	UFUNCTION(Server, Reliable)
	void Server_RequestBeginPlay(APlayerController* PlayerController);
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Server")
	void Server_SendNumber(const FString& NumString);

	

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_SendResult(FNumberBaseballResult Result);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_TurnStart(APlayerController* PlayerController);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_IsOut(APlayerController* PlayerController);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_IsWinner(APlayerController* PlayerController);
protected:
	virtual void BeginPlay() override;

private:
	float DisplayTime = 10.0f;
	float WarningDisplayTime = 5.0f;
	float ChatDisplayTime = 8.0f;

	FColor ChatColor = FColor::Green;
	FColor WarningColor = FColor::Red;
	FColor ServerColor = FColor::Emerald;
};
