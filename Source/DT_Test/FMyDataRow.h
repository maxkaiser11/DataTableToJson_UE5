// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FMyDataRow.generated.h"

USTRUCT(BlueprintType)
struct DT_TEST_API FMyDataRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Make;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Model;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Year;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;
};
