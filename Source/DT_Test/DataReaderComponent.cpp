// Fill out your copyright notice in the Description page of Project Settings.


#include "DataReaderComponent.h"

#include "DT_Test.h"
#include "FMyDataRow.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "JsonObjectConverter.h"


UDataReaderComponent::UDataReaderComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UDataReaderComponent::BeginPlay()
{
	Super::BeginPlay();

	const FString Json = DataTableToJsonString(true);
	PrintTableRows();
	SaveDataTableJsonToDisk(Json);
	
}


void UDataReaderComponent::PrintTableRows()
{
	FString JsonString = DataTableToJsonString(false);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, JsonString);
	}
}

FString UDataReaderComponent::DataTableToJsonString(bool bIncludeRowName) const
{
	if (!MyDataTable)
	{
		return TEXT("[]");
	}

	const UScriptStruct* RowStruct = MyDataTable->GetRowStruct();
	if (!RowStruct)
	{
		return TEXT("[]");
	}

	// Access the raw row map (RowName -> row bytes)
	const TMap<FName, uint8*>& RowMap = MyDataTable->GetRowMap();

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	JsonArray.Reserve(RowMap.Num());

	for (const TPair<FName, uint8*>& Pair : RowMap)
	{
		const FName& RowName = Pair.Key;
		const uint8* RowData = Pair.Value;

		TSharedRef<FJsonObject> RowObject = MakeShared<FJsonObject>();

		// Convert using the actual struct type
		if (!FJsonObjectConverter::UStructToJsonObject(RowStruct, RowData, RowObject, /*CheckFlags*/0, /*SkipFlags*/0))
		{
			UE_LOG(LogTemp, Warning, TEXT("UStructToJsonObject failed for row '%s'"), *RowName.ToString());
			continue;
		}

		if (bIncludeRowName)
		{
			RowObject->SetStringField(TEXT("RowName"), RowName.ToString());
		}

		JsonArray.Add(MakeShared<FJsonValueObject>(RowObject));
	}

	// Serialize the array (compact)
	FString OutJson;
	using FWriter = TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;
	TSharedRef<FWriter> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJson);
	FJsonSerializer::Serialize(JsonArray, Writer);

	return OutJson;
}

void UDataReaderComponent::SaveDataTableJsonToDisk(const FString& Json)
{
	const FString Path = FPaths::ProjectSavedDir() / TEXT("MyTable.json");
	FFileHelper::SaveStringToFile(Json, *Path);
}

