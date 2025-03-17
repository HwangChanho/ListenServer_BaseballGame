#include "NumberBaseballGameMode.h"
#include "NumberBaseballController.h"
#include "NumberBaseballGameState.h"

ANumberBaseballGameMode::ANumberBaseballGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANumberBaseballGameMode::BeginPlay()
{
	Super::BeginPlay();

	Manager = &FGameManager::Get();
}

void ANumberBaseballGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CurrentGameState == EGameState::GameOver)
	{
		BroadcastMessageToAllControllers("[Server] Waiting for player....");
		CurrentGameState = EGameState::Waiting;
		Manager->RemovePlayer();
	}

	const FString& TimerText = GetTimerCount();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(It->Get()))
		{
			CastingController->Client_SendTimeLeft(TimerText);
		}
	}
}

void ANumberBaseballGameMode::StartTurnTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		TurnTimerHandle,
		this,
		&ANumberBaseballGameMode::OnTurnTimeout,
		AvailTime,
		false
	);
}

void ANumberBaseballGameMode::OnTurnTimeout()
{
	CheckNumberString(Manager->GetCurrentPlayer(), "000");
}

void ANumberBaseballGameMode::CancelTurnTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(TurnTimerHandle);
}

const FString& ANumberBaseballGameMode::GetTimerCount()
{
	static FString TimerText;

	if (GetWorld()->GetTimerManager().IsTimerActive(TurnTimerHandle))
	{
		float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(TurnTimerHandle);
		TimerText = FString::Printf(TEXT("%.2f"), RemainingTime);
	}
	else
	{
		TimerText = TEXT("0");
	}

	return TimerText;
}

void ANumberBaseballGameMode::GenerateGame()
{
	BroadcastMessageToAllControllers("[Server] Game Generating....");
	Manager->AddReadyPlayer();

	if (Manager->GetPlayerCount() > 1) // 2명부터 게임 시작
	{
		Manager->SetCurrentPlayer();
		CurrentGameState = EGameState::Playing;
		GenerateRandNumber();
		AdvanceTurn();
	}
	else
	{
		BroadcastMessageUsingGameState("[Server] Player Not Enough");
	}
}

void ANumberBaseballGameMode::AdvanceTurn()
{
	StartTurnTimer();
	if (Manager->GetPlayerCount() == 0)
	{
		CancelTurnTimer();
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

	// 한명일때
	if (Manager->GetPlayerCount() == 1)
	{
		Manager->SetCurrentPlayer();
		APlayerController* BeginPlayer = Manager->GetPlayerByIndex(0);
		const FString Message = FString::Printf(TEXT("[Server] [%d] Turn"), BeginPlayer->GetUniqueID());
		BroadcastMessageToAllControllers(Message);
		
		if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(BeginPlayer))
		{
			CastingController->Client_TurnStart(BeginPlayer->GetUniqueID());
		}

		return;
	}
	
	Manager->AdvanceTurn(false);
	
	// 다음 플레이어에게 턴 시작 알림
	if (ANumberBaseballController* StartController = Cast<ANumberBaseballController>(Manager->GetCurrentPlayer()))
	{
		const int32 BeginPlayerID = StartController->GetUniqueID();
		const FString Message = FString::Printf(TEXT("[Server] [%d] Turn"), BeginPlayerID);
		BroadcastMessageToAllControllers(Message);
		
		for (const auto& Controller : Manager->GetPlayerOrder())
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
			if (PlayerController == WorldPlayerController)
			{
				if (!Manager->GetPlayerContains(WorldPlayerController))
				{
					Manager->ReadyPlayer(PlayerController);
					FString Log = FString::Format(TEXT("[Server] {0} ready"), { PlayerController->GetUniqueID() });
					BroadcastMessageToAllControllers(Log);

					if (Manager->GetReadyPlayerCount() > 1)
					{
						BroadcastMessageToAllControllers("[Server] Host must type '/start' to play game");
					}
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
	Manager->GenerateRandNum();
	UE_LOG(LogTemp, Warning, TEXT("Generated Number: %s"), *Manager->GetCorrectNumber());
}


void ANumberBaseballGameMode::ReceiveCheckResult(APlayerController* PlayerController)
{
	if (ANumberBaseballController* NumberBaseballPlayerController = Cast<ANumberBaseballController>(PlayerController))
	{
		if (Manager->CheckPlayerIsOut(PlayerController))
		{
			const int32 PlayerID = PlayerController->GetUniqueID();
			NumberBaseballPlayerController->Client_IsOut(PlayerID);
			NumberBaseballPlayerController->Client_SendResult(Manager->GetPlayerResult(PlayerController));
			
		}
		else
		{
			NumberBaseballPlayerController->Client_SendResult(Manager->GetPlayerResult(PlayerController));
		}
	}
	
	AdvanceTurn();
}

void ANumberBaseballGameMode::CheckNumberString(APlayerController* PlayerController, const FString& InputNumberString)
{
	if (EGameState::Playing != CurrentGameState || Manager->GetCurrentPlayer() != PlayerController) return;
	CancelTurnTimer();

	bool bIsCorrect = (InputNumberString.Len() == 3 && InputNumberString == Manager->GetCorrectNumber());
	
	if (InputNumberString == "000")
	{
		BroadcastMessageToAllControllers("Time Out!!!");
	}
	else
	{
		FString Log = FString::Format(TEXT("[Player {0}] {1} -> {2}"), { PlayerController->GetUniqueID(), InputNumberString, bIsCorrect ? TEXT("Correct") : TEXT("Wrong") });
		BroadcastMessageToAllControllers(Log);
	}

	Manager->UpdatePlayerScore(PlayerController, bIsCorrect);
	UE_LOG(LogTemp, Warning, TEXT("Generated Number: %s"), *Manager->GetCorrectNumber());
	
	const FNumberBaseballResult Result = Manager->GetPlayerResult(PlayerController);
	if (Result.StrikeCount >= WinCount)
	{
		int32 WonPlayerID = PlayerController->GetUniqueID();
		FString WinnerLog = FString::Format(TEXT("[Server] Player {0} Winner Winner!"), { WonPlayerID });
		BroadcastMessageToAllControllers(WinnerLog);
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(It->Get()))
			{
				if (PlayerController == CastingController)
				{
					Manager->SetWinnerMap(PlayerController);
				}
				
				CastingController->Client_IsWinner(Manager->GetWinnerScoreString(), true);
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


// 채팅 & 전역로그
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

void ANumberBaseballGameMode::BroadcastMessageUsingGameState(const FString& Message)
{
	if (ANumberBaseballGameState* MyGameState = GetGameState<ANumberBaseballGameState>())
	{
		MyGameState->MulticastBroadcastMessage(Message);
	}
}