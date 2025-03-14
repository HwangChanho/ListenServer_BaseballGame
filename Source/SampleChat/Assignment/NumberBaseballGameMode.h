#pragma once

#include "CoreMinimal.h"
#include "FNumberBaseballResult.h"
#include "GameFramework/GameMode.h"
#include "NumberBaseballGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Waiting UMETA(DisplayName = "Waiting"),
	Playing UMETA(DisplayName = "Playing"),
	GameOver UMETA(DisplayName = "Game Over")
};

UCLASS()
class SAMPLECHAT_API ANumberBaseballGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ANumberBaseballGameMode();

	// 플레이어 등록 등록시 게임이 시작된다.
	void PlayGame();
	// 랜덤번호 생성
	void GenerateRandNumber();
	// 플레이어가 입력한 번호를 검증
	void CheckNumberString(APlayerController* PlayerController, const FString& InputNumberString);
	// 검증 결과 전송
	void ReceiveCheckResult(APlayerController* PlayerController);
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	FString CorrectNumber;
	UPROPERTY()
	APlayerController* CurrentTurnPlayer;

	UPROPERTY()
	TMap<APlayerController*, FNumberBaseballResult> PlayerDetailMap; // 순서 보장 안됨
	UPROPERTY()
	TArray<APlayerController*> PlayerOrder;

	EGameState CurrentGameState = EGameState::GameOver;

	// 게임 시작
	void GenerateGame();
	// 턴을 넘겨준다
	void AdvanceTurn();
	// 플레이어 컨트롷러 가져오기
	void AddAllPlayerControllers();
};
