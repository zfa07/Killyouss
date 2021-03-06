// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/Actor.h"
#include "MrcCalibrationData.h" // for FMrcGarbageMatteSaveData
#include "MrcGarbageMatteCaptureComponent.generated.h"

class AMrcGarbageMatteActor;
class UMrcCalibrationData;

/**
 *	
 */
UCLASS(ClassGroup = Rendering, editinlinenew, config = Engine)
class MIXEDREALITYCAPTUREFRAMEWORK_API UMrcGarbageMatteCaptureComponent : public USceneCaptureComponent2D
{
	GENERATED_UCLASS_BODY()

public:
	//~ UActorComponent interface
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:
	//~ USceneCaptureComponent interface
	virtual const AActor* GetViewOwner() const override;

public:
	UFUNCTION()
	void SetTrackingOrigin(USceneComponent* TrackingOrigin);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MixedRealityCapture|GarbageMatting")
	void ApplyCalibrationData(const UMrcCalibrationData* ConfigData);

	UFUNCTION(BlueprintCallable, Category = "MixedRealityCapture|GarbageMatting")
	void SetGarbageMatteActor(AMrcGarbageMatteActor* NewActor);

	UFUNCTION(BlueprintCallable, Category = "MixedRealityCapture|GarbageMatting")
	void GetGarbageMatteData(TArray<FMrcGarbageMatteSaveData>& GarbageMatteDataOut);

	UFUNCTION(BlueprintNativeEvent, Category = "MixedRealityCapture|GarbageMatting")
	AMrcGarbageMatteActor* SpawnNewGarbageMatteActor(USceneComponent* TrackingOrigin);

private:
	UPROPERTY(Transient, Config)
	TSubclassOf<AMrcGarbageMatteActor> GarbageMatteActorClass;

	UPROPERTY(Transient)
	AMrcGarbageMatteActor* GarbageMatteActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<USceneComponent> TrackingOriginPtr;
};

/* AMrcGarbageMatteActor
 *****************************************************************************/

class UMaterial;
class UStaticMesh;

UCLASS(notplaceable, Blueprintable)
class AMrcGarbageMatteActor : public AActor
{
	GENERATED_UCLASS_BODY()


public: 
	UFUNCTION(BlueprintCallable, Category = "MixedRealityCapture|GarbageMatting")
	void ApplyCalibrationData(const TArray<FMrcGarbageMatteSaveData>& GarbageMatteData);

	UFUNCTION(BlueprintCallable, Category = "MixedRealityCapture|GarbageMatting")
	UPrimitiveComponent* AddNewGabageMatte(const FMrcGarbageMatteSaveData& GarbageMatteData);

	UFUNCTION(BlueprintNativeEvent, Category = "MixedRealityCapture|GarbageMatting")
	UPrimitiveComponent* CreateGarbageMatte(const FMrcGarbageMatteSaveData& GarbageMatteData);

	UFUNCTION(BlueprintCallable, Category = "MixedRealityCapture|GarbageMatting")
	void GetGarbageMatteData(TArray<FMrcGarbageMatteSaveData>& GarbageMatteDataOut);

private:
	UPROPERTY(Transient)
	UStaticMesh* GarbageMatteMesh;

	UPROPERTY(Transient)
	UMaterial* GarbageMatteMaterial;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MixedRealityCapture|GarbageMatting", meta=(AllowPrivateAccess="true"))
	TArray<UPrimitiveComponent*> GarbageMattes;
};
