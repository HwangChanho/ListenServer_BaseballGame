#pragma once

#include "CoreMinimal.h"
#include "FNumberBaseballResult.h"

class FGameManager
{
public:
	static FGameManager& Get()
	{
		static FGameManager Instance;
		return Instance;
	}

	// 복사 및 이동 금지
	FGameManager(const FGameManager&) = delete;
	FGameManager& operator=(const FGameManager&) = delete;
	FGameManager(FGameManager&&) = delete;
	FGameManager& operator=(FGameManager&&) = delete;

	void GenerateRandNum();
	void ReadyPlayer(APlayerController* PlayerController);
	void AddReadyPlayer();
	void RemovePlayer();
	void AdvanceTurn(bool bIsForce);
	void UpdatePlayerScore(APlayerController* PlayerController, bool bIsCorrect);
	
	bool CheckPlayerIsOut(APlayerController* PlayerController);
	
	FString GetWinnerScoreString();

	FORCEINLINE_DEBUGGABLE FString GetCorrectNumber() const { return CurrentNumber; }

	FORCEINLINE_DEBUGGABLE void SetCurrentPlayer() { CurrentTurnPlayer = PlayerOrder[0]; }
	FORCEINLINE_DEBUGGABLE void SetWinnerMap(APlayerController* PlayerController) { WinnerMap.FindOrAdd(PlayerController)++; }
	
	FORCEINLINE_DEBUGGABLE int GetPlayerCount() const { return PlayerOrder.Num(); }
	FORCEINLINE_DEBUGGABLE TArray<APlayerController*> GetPlayerOrder() const { return PlayerOrder; }
	FORCEINLINE_DEBUGGABLE APlayerController* GetPlayerByIndex(const int Index) const { return PlayerOrder[Index]; }
	
	FORCEINLINE_DEBUGGABLE int GetReadyPlayerCount() const { return PlayerDetailMap.Num(); }
	FORCEINLINE_DEBUGGABLE bool GetPlayerContains(const APlayerController* PlayerController) const { return PlayerDetailMap.Contains(PlayerController); }
	FORCEINLINE_DEBUGGABLE FNumberBaseballResult GetPlayerResult(const APlayerController* PlayerController) const { return PlayerDetailMap[PlayerController]; }
	
	FORCEINLINE_DEBUGGABLE APlayerController* GetCurrentPlayer() const { return CurrentTurnPlayer; }
private:
	FGameManager() {}
	~FGameManager() {}

	bool bTurnFlag = false;

	APlayerController* CurrentTurnPlayer;
	APlayerController* WinnerPlayer;

	FString CurrentNumber;

	TMap<APlayerController*, FNumberBaseballResult> PlayerDetailMap;
	TMap<APlayerController*, int32> WinnerMap;
	TArray<APlayerController*> PlayerOrder;
};
