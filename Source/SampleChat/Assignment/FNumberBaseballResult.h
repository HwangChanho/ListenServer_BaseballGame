#pragma once

#include "CoreMinimal.h"
#include "FNumberBaseballResult.generated.h"

USTRUCT(BlueprintType)
struct SAMPLECHAT_API FNumberBaseballResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseball")
	int BallCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseball")
	int StrikeCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseball")
	int TurnLeft;

	FNumberBaseballResult()
		: BallCount(0), StrikeCount(0), TurnLeft(3) {}
};
