#include "ChatUserWidget.h"

#include "ChatPlayerController.h"
#include "Components/Button.h"

void UChatUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SendButton)
	{
		SendButton->OnClicked.AddDynamic(this, &UChatUserWidget::OnSendButtonClicked);
	}

	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UChatUserWidget::OnLoginButtonClicked);
	}
}

void UChatUserWidget::Login()
{
	AChatPlayerController* Controller = Cast<AChatPlayerController>(GetOwningPlayer());
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
		return;
	}

	FString UserID = LoginTextField->GetText().ToString();
	if (UserID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Message is empty. Not sending to server."));
		return;
	}

	Controller->Server_LoginWithID(UserID);
}

void UChatUserWidget::SendMessage()
{
	AChatPlayerController* Controller = Cast<AChatPlayerController>(GetOwningPlayer());
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL"));
		return;
	}

	FString Message = SendTextField->GetText().ToString();
	UE_LOG(LogTemp, Log, TEXT("[Client] Sending message to server: %s"), *Message);
	Controller->SendMessageToServer(Message);
}

void UChatUserWidget::OnSendButtonClicked()
{
	SendMessage();
}

void UChatUserWidget::OnLoginButtonClicked()
{
	Login();
}