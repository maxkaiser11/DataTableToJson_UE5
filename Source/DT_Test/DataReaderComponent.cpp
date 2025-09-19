// Fill out your copyright notice in the Description page of Project Settings.


#include "DataReaderComponent.h"

#include "DT_Test.h"
#include "FMyDataRow.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "JsonObjectConverter.h"


// Sets default values for this component's properties
UDataReaderComponent::UDataReaderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDataReaderComponent::BeginPlay()
{
	Super::BeginPlay();

	const FString Json = DataTableToJsonString(true);
	if (GEngine)
	{
		// Show first 4 lines to avoid spamming (optional)
		const FString Preview = Json.Left(500);
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, Preview);
	}

	SaveDataTableJsonToDisk(Json);
	
}


// Called every frame
void UDataReaderComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UDataReaderComponent::PrintTableRows()
{
	if (!MyDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("No DataTable assigned!"));
		return;
	}

	static const FString Context(TEXT("MyDataTableContext"));

	// Get all rows AND their row names
	TArray<FName> RowNames = MyDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		if (FMyDataRow* Row = MyDataTable->FindRow<FMyDataRow>(RowName, Context))
		{
			FString Msg = FString::Printf(TEXT("Make: %s | Modle: %s | Speed: %.2f"), *Row->Make, *Row->Model, Row->Speed);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, Msg);
			}
		}
	}
}

FString UDataReaderComponent::DataTableToJsonString(bool bIncludeRowName) const
{
	if (!MyDataTable)
	{
		return TEXT("[]");
	}

	// Gather row names so we can include them in output
	const TArray<FName> RowNames = MyDataTable->GetRowNames();

	// We'll build a JSON array of objects
	TArray<TSharedPtr<FJsonValue>> JsonArray;

	static const FString Ctx(TEXT("MyDataTableToJson"));

	for (const FName& RowName : RowNames)
	{
		if (const FMyDataRow* Row = MyDataTable->FindRow<FMyDataRow>(RowName, Ctx))
		{
			// Convert the row struct to a JSON object
			TSharedRef<FJsonObject> RowObject = MakeShared<FJsonObject>();
			// Put the struct fields
			FJsonObjectConverter::UStructToJsonObject(
				FMyDataRow::StaticStruct(),
				Row,
				RowObject,
				/*CheckFlags*/0, /*SkipFlags*/0
			);

			// Optionally add the row key
			if (bIncludeRowName)
			{
				RowObject->SetStringField(TEXT("RowName"), RowName.ToString());
			}

			JsonArray.Add(MakeShared<FJsonValueObject>(RowObject));
		}
	}

	// Serialize the array to a compact JSON string
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

