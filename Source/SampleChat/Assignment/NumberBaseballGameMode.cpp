#include "NumberBaseballGameMode.h"
#include "DebugHelper.h"
#include "NumberBaseballController.h"

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
		UE_LOG(LogTemp, Warning, TEXT("[Server] Player Not Enough"));
	}
}

void ANumberBaseballGameMode::AdvanceTurn()
{
	if (PlayerOrder.Num() == 0) return;

	const int32 CurrentIndex = PlayerOrder.Find(CurrentTurnPlayer);
	if (CurrentIndex == INDEX_NONE) return;

	const int32 NextIndex = (CurrentIndex + 1) % PlayerOrder.Num();
	CurrentTurnPlayer = PlayerOrder[NextIndex];

	UE_LOG(LogTemp, Warning, TEXT("[Server] CurrentTurnPlayer: %p"), CurrentTurnPlayer);

	// 다음 플레이어에게 턴 시작 알림
	if (ANumberBaseballController* NextController = Cast<ANumberBaseballController>(CurrentTurnPlayer))
	{
		NextController->Client_TurnStart(NextController);
	}
}

void ANumberBaseballGameMode::AddAllPlayerControllers()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PlayerController = Iterator->Get())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Server] Found PlayerController: %p"), PlayerController);
			if (!PlayerDetailMap.Contains(PlayerController))
			{
				PlayerDetailMap.Add(PlayerController, FNumberBaseballResult());
			}
		}
	}

	GenerateGame();
}

void ANumberBaseballGameMode::PlayGame()
{
	if (EGameState::Waiting != CurrentGameState) return;

	AddAllPlayerControllers();
}

void ANumberBaseballGameMode::GenerateRandNumber()
{
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
			NumberBaseballPlayerController->Client_IsOut(NumberBaseballPlayerController);
			PlayerOrder.Remove(PlayerController);

			if (PlayerOrder.Num() == 1 && PlayerOrder.IsValidIndex(0))
			{
				// 승자 전송
				NumberBaseballPlayerController->Client_IsWinner(PlayerOrder[0]);
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
	UE_LOG(LogTemp, Warning, TEXT("Input: %s | Correct: %s | Match: %s"),
		*InputNumberString, *CorrectNumber, bIsCorrect ? TEXT("YES") : TEXT("NO"));

	bIsCorrect ? ++PlayerDetailMap[PlayerController].StrikeCount : ++PlayerDetailMap[PlayerController].BallCount;
	--PlayerDetailMap[PlayerController].TurnLeft;

	ReceiveCheckResult(PlayerController);
}