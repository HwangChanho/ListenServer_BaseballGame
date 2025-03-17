#include "NumberBaseballGameMode.h"
#include "DebugHelper.h"
#include "NumberBaseballController.h"
#include "NumberBaseballGameState.h"

ANumberBaseballGameMode::ANumberBaseballGameMode()
{
	PrimaryActorTick.bCanEverTick  = true;
}

void ANumberBaseballGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ANumberBaseballGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CurrentGameState == EGameState::GameOver)
	{
		BroadcastMessageToAllControllers("[Server] Waiting for player....");
		CurrentGameState = EGameState::Waiting;
		PlayerDetailMap.Empty();
		PlayerOrder.Empty();
	}
}

void ANumberBaseballGameMode::GenerateGame()
{
	BroadcastMessageToAllControllers("[Server] Game Generating....");
	for (const auto& Entry : PlayerDetailMap)
	{
		if (!PlayerOrder.Contains(Entry.Key))
		{
			PlayerOrder.Add(Entry.Key);
		}
	}

	if (PlayerOrder.Num() > 1) // 2명부터 게임 시작
	{
		CurrentTurnPlayer = PlayerOrder[0];
		CurrentGameState = EGameState::Playing;
		GenerateRandNumber();
		AdvanceTurn();
	}
	else
	{
		BroadcastMessageUsingGameState("[Server] Player Not Enough");
	}
}

void ANumberBaseballGameMode::BroadcastMessageUsingGameState(const FString& Message)
{
	if (ANumberBaseballGameState* MyGameState = GetGameState<ANumberBaseballGameState>())
	{
		MyGameState->MulticastBroadcastMessage(Message);
	}
}

void ANumberBaseballGameMode::AdvanceTurn()
{
	if (PlayerOrder.Num() == 0)
	{
		BroadcastMessageToAllControllers("No Winner");
		CurrentGameState = EGameState::GameOver;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(It->Get()))
			{
				CastingController->Client_IsWinner("", false);
			}
		}
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("CurrentTurnPlayer ==> [%d]"), CurrentTurnPlayer->GetUniqueID());

	// 한명일때
	if (PlayerOrder.Num() == 1)
	{
		CurrentTurnPlayer = PlayerOrder[0];
		const int32 BeginPlayerID = PlayerOrder[0]->GetUniqueID();
		const FString Message = FString::Printf(TEXT("[Server] [%d] Turn"), BeginPlayerID);
		BroadcastMessageToAllControllers(Message);
		
		if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(PlayerOrder[0]))
		{
			CastingController->Client_TurnStart(BeginPlayerID);
		}

		return;
	}

	const int32 CurrentIndex = PlayerOrder.Find(CurrentTurnPlayer);
	if (CurrentIndex == INDEX_NONE) return;

	const int32 NextIndex = (CurrentIndex + 1) % PlayerOrder.Num();
	CurrentTurnPlayer = PlayerOrder[NextIndex];
	
	// 다음 플레이어에게 턴 시작 알림
	if (ANumberBaseballController* StartController = Cast<ANumberBaseballController>(CurrentTurnPlayer))
	{
		const int32 BeginPlayerID = StartController->GetUniqueID();
		const FString Message = FString::Printf(TEXT("[Server] [%d] Turn"), BeginPlayerID);
		BroadcastMessageToAllControllers(Message);
		
		for (const auto& Controller : PlayerOrder)
		{
			if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(Controller))
			{
				CastingController->Client_TurnStart(BeginPlayerID);
			}
		}
	}
}

void ANumberBaseballGameMode::AddAllPlayerControllers(APlayerController* PlayerController)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* WorldPlayerController = Iterator->Get())
		{
			if (!PlayerDetailMap.Contains(PlayerController) && PlayerController == WorldPlayerController)
			{
				PlayerDetailMap.Add(PlayerController, FNumberBaseballResult());
				FString Log = FString::Format(TEXT("[Server] {0} ready"), { PlayerController->GetUniqueID() });
				BroadcastMessageToAllControllers(Log);

				if (PlayerDetailMap.Num() > 1)
				{
					BroadcastMessageToAllControllers("[Server] Host must type '/start' to play game");
				}
			}
		}
	}
}

void ANumberBaseballGameMode::PlayGame()
{
	if (EGameState::Waiting != CurrentGameState) return;

	GenerateGame();
}

void ANumberBaseballGameMode::PlayerReady(APlayerController* PlayerController)
{
	if (EGameState::Playing == CurrentGameState) return;

	AddAllPlayerControllers(PlayerController);
}

void ANumberBaseballGameMode::GenerateRandNumber()
{
	BroadcastMessageUsingGameState("[Server] Generating RandNumber");
	FString RandomNumber;
	
	while (RandomNumber.Len() < 3)
	{
		int32 RandNumber = FMath::RandRange(0, 9);
		
		if (!RandomNumber.Contains(FString::FromInt(RandNumber)))
		{
			RandomNumber += FString::FromInt(RandNumber);
		}
	}

	CorrectNumber = RandomNumber;
	UE_LOG(LogTemp, Warning, TEXT("Generated Number: %s"), *CorrectNumber);
}

void ANumberBaseballGameMode::ReceiveCheckResult(APlayerController* PlayerController)
{
	if (PlayerDetailMap[PlayerController].TurnLeft < 0 || PlayerDetailMap[PlayerController].BallCount > 2)
	{
		if (ANumberBaseballController* NumberBaseballPlayerController = Cast<ANumberBaseballController>(PlayerController))
		{
			int32 PlayerID = PlayerController->GetUniqueID();
			NumberBaseballPlayerController->Client_IsOut(PlayerID);
			PlayerOrder.Remove(PlayerController);
			NumberBaseballPlayerController->Client_SendResult(PlayerDetailMap[PlayerController]);
		}
	}
	else
	{
		if (ANumberBaseballController* NumberBaseballPlayerController = Cast<ANumberBaseballController>(PlayerController))
		{
			NumberBaseballPlayerController->Client_SendResult(PlayerDetailMap[PlayerController]);
		}
	}

	AdvanceTurn();
}

void ANumberBaseballGameMode::CheckNumberString(APlayerController* PlayerController, const FString& InputNumberString)
{
	if (EGameState::Playing != CurrentGameState || CurrentTurnPlayer != PlayerController) return;

	bool bIsCorrect = (InputNumberString.Len() == 3 && InputNumberString == CorrectNumber);
	FString Log = FString::Format(TEXT("[Player {0}] {1} -> {2}"),
	{ PlayerController->GetUniqueID(), InputNumberString, bIsCorrect ? TEXT("Correct") : TEXT("Wrong") });
	BroadcastMessageToAllControllers(Log);

	bIsCorrect ? ++PlayerDetailMap[PlayerController].StrikeCount : ++PlayerDetailMap[PlayerController].BallCount;
	--PlayerDetailMap[PlayerController].TurnLeft;

	if (PlayerDetailMap[PlayerController].StrikeCount > WinCount)
	{
		int32 WonPlayerID = PlayerController->GetUniqueID();
		FString WinnerLog = FString::Format(TEXT("[Server] Player {0} Winner Winner!"), { WonPlayerID });
		BroadcastMessageToAllControllers(WinnerLog);
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(It->Get()))
			{
				if (PlayerController)
				{
					WinnerMap.FindOrAdd(PlayerController)++;
				}

				FString ScoreString = "";
				for (const auto& Score : WinnerMap)
				{
					FString Winner = FString::Printf(TEXT("Player %d: %d\n"), Score.Key->GetUniqueID(), Score.Value);
					ScoreString += Winner;
				}
				
				CastingController->Client_IsWinner(ScoreString, true);
				CurrentGameState = EGameState::GameOver;
			}
		}
		return;
	}

	ReceiveCheckResult(PlayerController);
}

void ANumberBaseballGameMode::PlayerWin()
{
	
}




// 채팅

void ANumberBaseballGameMode::BroadcastMessageToAllControllers(const FString& Message)
{
	Multicast_BroadcastMessage(Message);
}

void ANumberBaseballGameMode::Multicast_BroadcastMessage_Implementation(const FString& Message)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(It->Get()))
		{
			CastingController->Client_ReceiveMessage(Message);
		}
	}
}