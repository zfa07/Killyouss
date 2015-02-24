// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AIModulePrivate.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Distance.h"

namespace
{
	FORCEINLINE float CalcDistance3D(const FVector& PosA, const FVector& PosB)
	{
		return (PosB - PosA).Size();
	}

	FORCEINLINE float CalcDistance2D(const FVector& PosA, const FVector& PosB)
	{
		return (PosB - PosA).Size2D();
	}

	FORCEINLINE float CalcDistanceZ(const FVector& PosA, const FVector& PosB)
	{
		return PosB.Z - PosA.Z;
	}
}

UEnvQueryTest_Distance::UEnvQueryTest_Distance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DistanceTo = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UEnvQueryTest_Distance::RunTest(FEnvQueryInstance& QueryInstance) const
{
	FloatValueMin.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();

	// don't support context Item here, it doesn't make any sense
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(DistanceTo, ContextLocations))
	{
		return;
	}

	switch (TestMode)
	{
	case EEnvTestDistance::Distance3D:	
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const float Distance = CalcDistance3D(ItemLocation, ContextLocations[ContextIndex]);
				It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
			}
		}
		break;
	case EEnvTestDistance::Distance2D:	
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const float Distance = CalcDistance2D(ItemLocation, ContextLocations[ContextIndex]);
				It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
			}
		}
		break;
	case EEnvTestDistance::DistanceZ:	
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, *It);
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const float Distance = CalcDistanceZ(ItemLocation, ContextLocations[ContextIndex]);
				It.SetScore(TestPurpose, FilterType, Distance, MinThresholdValue, MaxThresholdValue);
			}
		}
		break;
	default:
		return;
	}
}

FString UEnvQueryTest_Distance::GetDescriptionTitle() const
{
	FString ModeDesc;
	switch (TestMode)
	{
	case EEnvTestDistance::Distance3D:	ModeDesc = TEXT(""); break;
	case EEnvTestDistance::Distance2D:	ModeDesc = TEXT(" 2D"); break;
	case EEnvTestDistance::DistanceZ:	ModeDesc = TEXT(" Z"); break;
	default: break;
	}

	return FString::Printf(TEXT("%s%s: to %s"), 
		*Super::GetDescriptionTitle(), *ModeDesc,
		*UEnvQueryTypes::DescribeContext(DistanceTo).ToString());
}

FText UEnvQueryTest_Distance::GetDescriptionDetails() const
{
	return DescribeFloatTestParams();
}
