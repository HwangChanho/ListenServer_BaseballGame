#include "ChatGameModeBase.h"
#include "ChatPlayerController.h"


AChatGameModeBase::AChatGameModeBase()
{
	
}

// Login
FString AChatGameModeBase::GetPlayerID(APlayerController* PlayerController)
{
	if (PlayerIDMap.Contains(PlayerController))
	{
		return *PlayerIDMap.Find(PlayerController);
	}
	return TEXT("");
}

void AChatGameModeBase::RemovePlayer(APlayerController* PlayerController)
{
	if (PlayerIDMap.Contains(PlayerController))
	{
		PlayerIDMap.Remove(PlayerController);
		UE_LOG(LogTemp, Log, TEXT("Removed PlayerController %p from map"), PlayerController);
	}
}

bool AChatGameModeBase::VerifyLogin(APlayerController* PlayerController, const FString& UserID)
{
	if (UserID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VerifyLogin] Login failed: User ID is empty"));
		return false;
	}

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VerifyLogin] Login failed: Controller is NULL"));
		return false;
	}

	if (PlayerIDMap.Contains(PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VerifyLogin] Login failed: PlayerController is already in the map"));
		return false;
	}

	PlayerIDMap.Add(PlayerController, UserID);
	
	return true;
}

// Message
void AChatGameModeBase::GotMessageFromClient_Implementation(APlayerController* PlayerController, const FString& Message)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("[GotMessageFromClient] PlayerController is NULL"));
		return;
	}

	if (!PlayerIDMap.Contains(PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GotMessageFromClient] Player is not in the map"));
		return;
	}

	FString PlayerID = GetPlayerID(PlayerController);
	FString FormattedMessage = FString::Printf(TEXT("[%s]: %s"), *PlayerID, *Message);

	if (!PlayerID.IsEmpty())
	{
		if (AChatPlayerController* ChatPC = Cast<AChatPlayerController>(PlayerController))
		{
			ChatPC->Client_ReceiveMessage(FormattedMessage);
		}
	}
}

bool AChatGameModeBase::GotMessageFromClient_Validate(APlayerController* PlayerController, const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("[Server][GotMessageFromClient_Validate][%s]"), *Message);
	return true;  // 기본적으로 모든 메시지를 허용 (보안이 필요하면 여기서 필터링 가능)
}