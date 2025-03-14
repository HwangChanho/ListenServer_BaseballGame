#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChatPlayerController.generated.h"

UCLASS()
class SAMPLECHAT_API AChatPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Login")
	void Server_LoginWithID(const FString& UserID);

	UFUNCTION(Client, Reliable)
	void Client_LoginSuccess(const FString& UserID);


	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "Message")
	void SendMessageToServer(const FString& Message);

	UFUNCTION(Client, Unreliable)
	void Client_ReceiveMessage(const FString& Message);

	void Login(const FString& UserID);

protected:
	virtual void BeginPlay() override;
};
