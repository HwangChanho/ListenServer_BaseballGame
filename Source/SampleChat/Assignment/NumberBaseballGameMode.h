#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameManager.h"
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
	// 플레이어 준비 등록
	void PlayerReady(APlayerController* PlayerController);
	// 자신의 턴인지 확인한다.
	FORCEINLINE_DEBUGGABLE bool CheckTurn(const APlayerController* PlayerController) const { return Manager->GetCurrentPlayer() == PlayerController; }
	// 랜덤번호 생성
	void GenerateRandNumber();
	// 플레이어가 입력한 번호를 검증
	void CheckNumberString(APlayerController* PlayerController, const FString& InputNumberString);
	// 검증 결과 전송
	void ReceiveCheckResult(APlayerController* PlayerController);
	// 클라이언트에 로그 전송
	void BroadcastMessageUsingGameState(const FString& Message);

	// 클라이언트에 메세지 전송
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastMessage(const FString& Message);
	void BroadcastMessageToAllControllers(const FString& Message);
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	FGameManager* Manager; // 게임 매니져
	EGameState CurrentGameState = EGameState::GameOver;

	FTimerHandle TurnTimerHandle;

	int WinCount = 3;
	float AvailTime = 10.0f;

	// 게임 시작
	void GenerateGame();
	// 턴을 넘겨준다
	void AdvanceTurn();
	// 플레이어 컨트롷러 가져오기
	void AddAllPlayerControllers(APlayerController* PlayerController);

	// 타이머 관련
	void StartTurnTimer();
	void OnTurnTimeout();
	void CancelTurnTimer();
	const FString& GetTimerCount();
};