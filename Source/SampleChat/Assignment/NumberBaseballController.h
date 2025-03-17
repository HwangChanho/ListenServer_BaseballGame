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

	UFUNCTION(Server, Reliable)
	void Server_RequestReady(APlayerController* PlayerController);
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Server")
	void Server_SendNumber(APlayerController* PlayerController, const FString& NumString);

	// 서버에 메세지 전송
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Server")
	void Server_SendMessage(const FString& Message);

	// 서버로부터 메세지 응답
	UFUNCTION(Client, Reliable)
	void Client_ReceiveMessage(const FString& Message);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_SendResult(FNumberBaseballResult Result);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_TurnStart(int32 PlayerID);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_IsOut(int32 PlayerID);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Client")
	void Client_IsWinner(const FString& ScoreString, bool bIsWinner); // 큰 값음 참조로 전달해야 네트워크 복제에 문제 안생김
	
	UFUNCTION(BlueprintCallable)
	void SendMessageToServer(const FString& Message);
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	float DisplayTime = 10.0f;
	float WarningDisplayTime = 5.0f;
	float ChatDisplayTime = 8.0f;

	FColor ChatColor = FColor::Green;
	FColor WarningColor = FColor::Red;
	FColor ServerColor = FColor::Emerald;

	void SetCurrentPlayer() const;
};
