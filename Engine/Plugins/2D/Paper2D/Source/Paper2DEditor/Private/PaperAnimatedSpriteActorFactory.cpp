// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Paper2DEditorPrivatePCH.h"
#include "AssetData.h"

//////////////////////////////////////////////////////////////////////////
// UPaperFlipbookActorFactory

UPaperFlipbookActorFactory::UPaperFlipbookActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Paper2D", "PaperFlipbookFactoryDisplayName", "Add Animated Sprite");
	NewActorClass = APaperFlipbookActor::StaticClass();
}

void UPaperFlipbookActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	if (UPaperFlipbook* Flipbook = Cast<UPaperFlipbook>(Asset))
	{
		GEditor->SetActorLabelUnique(NewActor, Flipbook->GetName());

		APaperFlipbookActor* TypedActor = CastChecked<APaperFlipbookActor>(NewActor);
		UPaperFlipbookComponent* RenderComponent = TypedActor->GetRenderComponent();
		check(RenderComponent);

		RenderComponent->UnregisterComponent();
		RenderComponent->SetFlipbook(Flipbook);
		RenderComponent->RegisterComponent();
	}
}

void UPaperFlipbookActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (UPaperFlipbook* Flipbook = Cast<UPaperFlipbook>(Asset))
	{
		if (APaperFlipbookActor* TypedActor = Cast<APaperFlipbookActor>(CDO))
		{
			UPaperFlipbookComponent* RenderComponent = TypedActor->GetRenderComponent();
			check(RenderComponent);

			RenderComponent->SetFlipbook(Flipbook);
		}
	}
}

bool UPaperFlipbookActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(UPaperFlipbook::StaticClass()))
	{
		return true;
	}
	else
	{
		OutErrorMsg = NSLOCTEXT("Paper2D", "CanCreateActorFrom_NoFlipbook", "No flipbook was specified.");
		return false;
	}
}
