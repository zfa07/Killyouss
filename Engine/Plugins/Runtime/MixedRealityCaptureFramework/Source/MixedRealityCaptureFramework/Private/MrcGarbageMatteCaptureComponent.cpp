// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MrcGarbageMatteCaptureComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "MrcCalibrationData.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "MrcUtilLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "MrcFrameworkSettings.h"

/* UMrcGarbageMatteCaptureComponent
 *****************************************************************************/

//------------------------------------------------------------------------------
UMrcGarbageMatteCaptureComponent::UMrcGarbageMatteCaptureComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCaptureEveryFrame = true;
	PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
	PostProcessBlendWeight = 0.0f;
	ShowFlags.SetAtmosphericFog(false);
	ShowFlags.SetFog(false);

	const UMrcFrameworkSettings* MrcSettings = GetDefault<UMrcFrameworkSettings>();
	ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> DefaultTarget(*MrcSettings->DefaulGarbageMatteTarget.ToString());
	TextureTarget = DefaultTarget.Object;

	GarbageMatteActorClass = AMrcGarbageMatteActor::StaticClass();
}

//------------------------------------------------------------------------------
void UMrcGarbageMatteCaptureComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (GarbageMatteActor)
	{
		GarbageMatteActor->Destroy();
		GarbageMatteActor = nullptr;
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

//------------------------------------------------------------------------------
const AActor* UMrcGarbageMatteCaptureComponent::GetViewOwner() const
{
	// This lets SetOnlyOwnerSee on the garbage matte actor make it visible only to this capture component.
	// Basically the "owner" Actor's pointer is being used as an ID for who renders the actor.
	return GarbageMatteActor;
}

//------------------------------------------------------------------------------
void UMrcGarbageMatteCaptureComponent::SetTrackingOrigin(USceneComponent* InTrackingOrigin)
{
	TrackingOriginPtr = InTrackingOrigin;
	
	if (GarbageMatteActor)
	{
		GarbageMatteActor->AttachToComponent(InTrackingOrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		// have to reset the relative offset manually in case the actor was already attached to this component (result from a component destroy)
		GarbageMatteActor->SetActorRelativeTransform(FTransform::Identity);
	}
}

//------------------------------------------------------------------------------
void UMrcGarbageMatteCaptureComponent::ApplyCalibrationData_Implementation(const UMrcCalibrationData* ConfigData)
{
	if (ConfigData)
	{
		if (GarbageMatteActor == nullptr)
		{
			SetGarbageMatteActor( SpawnNewGarbageMatteActor(TrackingOriginPtr.Get()) );
		}

		FOVAngle = ConfigData->LensData.FOV;

		if (ensure(GarbageMatteActor))
		{
			GarbageMatteActor->ApplyCalibrationData(ConfigData->GarbageMatteSaveDatas);
		}
	}
}

//------------------------------------------------------------------------------
void UMrcGarbageMatteCaptureComponent::GetGarbageMatteData(TArray<FMrcGarbageMatteSaveData>& GarbageMatteDataOut)
{
	if (GarbageMatteActor)
	{
		GarbageMatteActor->GetGarbageMatteData(GarbageMatteDataOut);
	}
	else
	{
		GarbageMatteDataOut.Empty();
	}
}

//------------------------------------------------------------------------------
AMrcGarbageMatteActor* UMrcGarbageMatteCaptureComponent::SpawnNewGarbageMatteActor_Implementation(USceneComponent* InTrackingOrigin)
{
	AMrcGarbageMatteActor* NewGarbageMatteActor = nullptr;

	UWorld* MyWorld = GetWorld();
#if WITH_EDITORONLY_DATA
	if (MyWorld && MyWorld->IsGameWorld())
#endif
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = TEXT("MR_GarbageMatteActor");

		UClass* SpawnClass = GarbageMatteActorClass;
		if (SpawnClass == nullptr)
		{
			SpawnClass = AMrcGarbageMatteActor::StaticClass();
		}

		AActor* SpawnedActor = MyWorld->SpawnActor(GarbageMatteActorClass, /*Location =*/nullptr, /*Rotation =*/nullptr, SpawnParameters);
		if (ensure(SpawnedActor) && InTrackingOrigin) 
		{
			SpawnedActor->AttachToComponent(InTrackingOrigin, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
		NewGarbageMatteActor = CastChecked<AMrcGarbageMatteActor>(SpawnedActor, ECastCheckedType::NullAllowed);
	}
	return NewGarbageMatteActor;
}

//------------------------------------------------------------------------------
void UMrcGarbageMatteCaptureComponent::SetGarbageMatteActor(AMrcGarbageMatteActor* NewActor)
{
	TArray<FMrcGarbageMatteSaveData> GarbageMatteData;
	if (GarbageMatteActor)
	{
		GarbageMatteActor->GetGarbageMatteData(GarbageMatteData);

		ShowOnlyActors.Remove(GarbageMatteActor);
		GarbageMatteActor->Destroy();
		GarbageMatteActor = nullptr;
	}

	GarbageMatteActor = NewActor;

	if (NewActor)
	{
		ShowOnlyActors.Add(NewActor);
		NewActor->ApplyCalibrationData(GarbageMatteData);

		if (TrackingOriginPtr.IsValid())
		{
			NewActor->AttachToComponent(TrackingOriginPtr.Get(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}
}

/* AMrcGarbageMatteActor
 *****************************************************************************/

//------------------------------------------------------------------------------
AMrcGarbageMatteActor::AMrcGarbageMatteActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	const UMrcFrameworkSettings* MrcSettings = GetDefault<UMrcFrameworkSettings>();

	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultGarbageMatteMesh;
		ConstructorHelpers::FObjectFinder<UMaterial>   DefaultGarbageMatteMaterial;

		FConstructorStatics(const UMrcFrameworkSettings* InMrcSettings)
			: DefaultGarbageMatteMesh(*InMrcSettings->DefaulGarbageMatteMesh.ToString())
			, DefaultGarbageMatteMaterial(*InMrcSettings->DefaulGarbageMatteMaterial.ToString())
		{}
	};
	static FConstructorStatics ConstructorStatics(MrcSettings);

	GarbageMatteMesh = ConstructorStatics.DefaultGarbageMatteMesh.Object;
	GarbageMatteMaterial = ConstructorStatics.DefaultGarbageMatteMaterial.Object;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TrackingOriginPt"));
}

//------------------------------------------------------------------------------
void AMrcGarbageMatteActor::ApplyCalibrationData(const TArray<FMrcGarbageMatteSaveData>& GarbageMatteData)
{
	for (UPrimitiveComponent* OldGarbageMatte : GarbageMattes)
	{
		OldGarbageMatte->DestroyComponent();
	}
	GarbageMattes.Empty(GarbageMatteData.Num());

	for (const FMrcGarbageMatteSaveData& Data : GarbageMatteData)
	{
		AddNewGabageMatte(Data);
	}
}

//------------------------------------------------------------------------------
UPrimitiveComponent* AMrcGarbageMatteActor::AddNewGabageMatte(const FMrcGarbageMatteSaveData& GarbageMatteData)
{
	UPrimitiveComponent* NewMatte = CreateGarbageMatte(GarbageMatteData);
	GarbageMattes.Add(NewMatte);

	return NewMatte;
}

//------------------------------------------------------------------------------
UPrimitiveComponent* AMrcGarbageMatteActor::CreateGarbageMatte_Implementation(const FMrcGarbageMatteSaveData& GarbageMatteData)
{
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, NAME_None);
	MeshComponent->SetStaticMesh(GarbageMatteMesh);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);
	MeshComponent->SetRelativeTransform(GarbageMatteData.Transform);
	MeshComponent->SetMaterial(0, GarbageMatteMaterial);
	MeshComponent->SetOnlyOwnerSee(true);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->RegisterComponent();

	return MeshComponent;
}

//------------------------------------------------------------------------------
void AMrcGarbageMatteActor::GetGarbageMatteData(TArray<FMrcGarbageMatteSaveData>& GarbageMatteDataOut)
{
	GarbageMatteDataOut.Empty(GarbageMattes.Num());

	FMrcGarbageMatteSaveData GarbageMatteData;
	for (const UPrimitiveComponent* GarbageMatte : GarbageMattes)
	{
		GarbageMatteData.Transform = GarbageMatte->GetRelativeTransform();
		GarbageMatteDataOut.Add(GarbageMatteData);
	}
}
