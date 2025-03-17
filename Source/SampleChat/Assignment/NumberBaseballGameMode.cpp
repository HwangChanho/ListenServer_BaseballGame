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
		const FString JoinLog = FString::Printf(TEXT("Waiting for player...."));
		DebugHelper::PrintDebugMessage(JoinLog, 1.0f, FColor::Cyan);
		CurrentGameState = EGameState::Waiting;
	}
}

void ANumberBaseballGameMode::GenerateGame()
{
	const FString JoinLog = FString::Printf(TEXT("Game Generating...."));
	DebugHelper::PrintDebugMessage(JoinLog, 1.0f, FColor::Cyan);
	
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
	if (PlayerOrder.Num() == 0) return;

	const int32 CurrentIndex = PlayerOrder.Find(CurrentTurnPlayer);
	if (CurrentIndex == INDEX_NONE) return;

	const int32 NextIndex = (CurrentIndex + 1) % PlayerOrder.Num();
	CurrentTurnPlayer = PlayerOrder[NextIndex];
	
	// 다음 플레이어에게 턴 시작 알림
	if (ANumberBaseballController* StartController = Cast<ANumberBaseballController>(CurrentTurnPlayer))
	{
		const int32 BeginPlayerID = StartController->GetUniqueID();
		const FString Message = FString::Printf(TEXT("[Server] [%d] Turn"), BeginPlayerID);
		BroadcastMessageUsingGameState(Message);
		
		for (const auto& c : PlayerOrder)
		{
			if (ANumberBaseballController* CastingController = Cast<ANumberBaseballController>(c))
			{
				CastingController->Client_TurnStart(BeginPlayerID, PlayerDetailMap[StartController]);
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
				UE_LOG(LogTemp, Warning, TEXT("[Server] Adding PlayerController: %p"), PlayerController);
				PlayerDetailMap.Add(PlayerController, FNumberBaseballResult());
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
	if (ANumberBaseballController* NumberBaseballPlayerController = Cast<ANumberBaseballController>(PlayerController))
	{
		if (PlayerDetailMap[PlayerController].TurnLeft < 0 || PlayerDetailMap[PlayerController].BallCount > 2)
		{
			int32 PlayerID = NumberBaseballPlayerController->GetUniqueID();
			NumberBaseballPlayerController->Client_IsOut(PlayerID);
			PlayerOrder.Remove(PlayerController);

			if (PlayerOrder.Num() == 1 && PlayerOrder.IsValidIndex(0))
			{
				// 승자 전송
				int32 WonPlayerID = PlayerOrder[0]->GetUniqueID();
				NumberBaseballPlayerController->Client_IsWinner(WonPlayerID);
				CurrentGameState = EGameState::GameOver;
			}
			return;
		}
		
		NumberBaseballPlayerController->Client_SendResult(PlayerDetailMap[PlayerController]);
		AdvanceTurn();
	}
}

void ANumberBaseballGameMode::CheckNumberString(APlayerController* PlayerController, const FString& InputNumberString)
{
	if (EGameState::Playing != CurrentGameState || CurrentTurnPlayer != PlayerController) return;

	bool bIsCorrect = (InputNumberString.Len() == 3 && InputNumberString == CorrectNumber);
	FString Log = FString::Format(TEXT("Input: {0} | Correct: {1} | Match: {2}"),
	{ InputNumberString, CorrectNumber, bIsCorrect ? TEXT("YES") : TEXT("NO") });
	BroadcastMessageToAllControllers(Log);

	bIsCorrect ? ++PlayerDetailMap[PlayerController].StrikeCount : ++PlayerDetailMap[PlayerController].BallCount;
	--PlayerDetailMap[PlayerController].TurnLeft;

	ReceiveCheckResult(PlayerController);
}

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