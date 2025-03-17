#include "GameManager.h"

void FGameManager::GenerateRandNum()
{
	constexpr int32 NumLength = 3;
	FString RandomNumber;
	const FRandomStream RandStream(FDateTime::Now().GetTicks());

	while (RandomNumber.Len() < NumLength)
	{
		int32 RandNumber = RandStream.RandRange(1, 9);
		
		if (!RandomNumber.Contains(FString::FromInt(RandNumber)))
		{
			RandomNumber += FString::FromInt(RandNumber);
		}
	}

	CurrentNumber = RandomNumber;
}

FString FGameManager::GetWinnerScoreString()
{
	FString ScoreString = "";
	for (const auto& Score : WinnerMap)
	{
		FString Winner = FString::Printf(TEXT("Player %d: %d\n"), Score.Key->GetUniqueID(), Score.Value);
		ScoreString += Winner;
	}
	return ScoreString;
}

bool FGameManager::CheckPlayerIsOut(APlayerController* PlayerController)
{
	if (PlayerDetailMap[PlayerController].BallCount > 2)
	{
		PlayerOrder.Remove(PlayerController);
		return true;
	}
	return false;
}

void FGameManager::UpdatePlayerScore(APlayerController* PlayerController, bool bIsCorrect)
{
	--PlayerDetailMap[PlayerController].TurnLeft;

	if (PlayerDetailMap[PlayerController].TurnLeft == 0) // 턴넘어가는 조건 1: 모든 턴 소진
	{
		++PlayerDetailMap[PlayerController].BallCount;
		PlayerDetailMap[CurrentTurnPlayer].TurnLeft = 3;
		bTurnFlag = true;
	}

	if (bIsCorrect) // 턴넘어가는 조건 2: 정답을 맞췄을때
	{
		++PlayerDetailMap[PlayerController].StrikeCount;
		PlayerDetailMap[PlayerController].BallCount = 0;
		PlayerDetailMap[PlayerController].TurnLeft = 3;
		bTurnFlag = true;
		GenerateRandNum();
	}
}

void FGameManager::RemovePlayer()
{
	PlayerDetailMap.Empty();
	PlayerOrder.Empty();
}

void FGameManager::AdvanceTurn(bool bIsForce)
{
	if (!bTurnFlag) return;
	bTurnFlag = false;
	
	const int32 CurrentIndex = PlayerOrder.Find(CurrentTurnPlayer);
	if (CurrentIndex == INDEX_NONE) return;

	const int32 NextIndex = (CurrentIndex + 1) % PlayerOrder.Num();
	CurrentTurnPlayer = PlayerOrder[NextIndex];
	
	if (bIsForce)
	{
		PlayerDetailMap[CurrentTurnPlayer].TurnLeft = 3;
		++PlayerDetailMap[CurrentTurnPlayer].BallCount;
	}
}

void FGameManager::ReadyPlayer(APlayerController* PlayerController)
{
	PlayerDetailMap.Add(PlayerController, FNumberBaseballResult());
}

void FGameManager::AddReadyPlayer()
{
	for (const auto& Entry : PlayerDetailMap)
	{
		if (!PlayerOrder.Contains(Entry.Key))
		{
			PlayerOrder.Add(Entry.Key);
		}
	}
}
