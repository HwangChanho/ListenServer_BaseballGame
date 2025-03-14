#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ChatGameModeBase.generated.h"

UCLASS()
class SAMPLECHAT_API AChatGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AChatGameModeBase();

	// 로그인 검증
	UFUNCTION(BlueprintCallable, Category = "Login")
	bool VerifyLogin(APlayerController* PlayerController, const FString& UserID);
	
	// 클라이언트에서 호출할 서버 RPC 함수
	UFUNCTION(Server, Reliable, WithValidation)
	void GotMessageFromClient(APlayerController* PlayerController, const FString& Message);

private:
	UPROPERTY()
	TMap<APlayerController*, FString> PlayerIDMap;

	void RemovePlayer(APlayerController* PlayerController);
	FString GetPlayerID(APlayerController* PlayerController);
};
