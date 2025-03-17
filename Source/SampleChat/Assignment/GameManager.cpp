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
	if (PlayerDetailMap[PlayerController].TurnLeft < 0 || PlayerDetailMap[PlayerController].BallCount > 2)
	{
		PlayerOrder.Remove(PlayerController);
		return true;
	}
	return false;
}

void FGameManager::UpdatePlayerScore(APlayerController* PlayerController, bool bIsCorrect)
{
	bIsCorrect ? ++PlayerDetailMap[PlayerController].StrikeCount : ++PlayerDetailMap[PlayerController].BallCount;
	--PlayerDetailMap[PlayerController].TurnLeft;

	if (bIsCorrect)
	{
		GenerateRandNum();
	}
}

void FGameManager::RemovePlayer()
{
	PlayerDetailMap.Empty();
	PlayerOrder.Empty();
}

void FGameManager::AdvanceTurn()
{
	const int32 CurrentIndex = PlayerOrder.Find(CurrentTurnPlayer);
	if (CurrentIndex == INDEX_NONE) return;

	const int32 NextIndex = (CurrentIndex + 1) % PlayerOrder.Num();
	CurrentTurnPlayer = PlayerOrder[NextIndex];
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
