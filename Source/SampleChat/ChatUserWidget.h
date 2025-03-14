#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "ChatUserWidget.generated.h"

class UButton;

UCLASS()
class SAMPLECHAT_API UChatUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UEditableText* LoginTextField;

	UPROPERTY(meta = (BindWidget))
	UEditableText* SendTextField;
	
	UPROPERTY(meta = (BindWidget))
	UButton* LoginButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SendButton;

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SendMessage();

	UFUNCTION(BlueprintCallable, Category = "Button")
	void Login();
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnSendButtonClicked();

	UFUNCTION()
	void OnLoginButtonClicked();
};
