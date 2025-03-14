#include "ChatPlayerController.h"
#include "ChatGameModeBase.h"

void AChatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

// Message
void AChatPlayerController::SendMessageToServer_Implementation(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[Server] Processing message from client: %s"), *Message);

	AChatGameModeBase* GameMode = Cast<AChatGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->GotMessageFromClient(this, Message);
	}
}

void AChatPlayerController::Client_ReceiveMessage_Implementation(const FString& Message)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, Message);
}

// Login
void AChatPlayerController::Login(const FString& UserID)
{
	Server_LoginWithID(UserID);
}

void AChatPlayerController::Server_LoginWithID_Implementation(const FString& UserID)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] Not a server! This should not happen."));
		return;
	}

	AChatGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AChatGameModeBase>();
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] GameMode is NULL!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Server] Player %s is trying to log in..."), *UserID);

	// 서버에서 로그인 검증
	if (!GameMode->VerifyLogin(this, UserID))
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] Login Failed for %s!"), *UserID);
		return;
	}

	// 로그인 성공 → 클라이언트에게 알림
	UE_LOG(LogTemp, Log, TEXT("[Server] Login Success for %s!"), *UserID);
	Client_LoginSuccess(UserID);
}

bool AChatPlayerController::Server_LoginWithID_Validate(const FString& UserID)
{
	return !UserID.IsEmpty(); // 유효성 검사
}

void AChatPlayerController::Client_LoginSuccess_Implementation(const FString& UserID)
{
	FString FormattedMessage = FString::Printf(TEXT("[%s] Login Success!"), *UserID);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, FormattedMessage);
}
