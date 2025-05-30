// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

#pragma once
#include "Engine/DataTable.h"
#include "InputIconMapping.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FInputIconMapping : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINav Input")
	TSoftObjectPtr<class UTexture2D> InputIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINav Input")
	TSoftObjectPtr<class UFont> InputIconFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINav Input")
	FString InputIconFontTextSolid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UINav Input")
	FString InputIconFontTextOutline;
};