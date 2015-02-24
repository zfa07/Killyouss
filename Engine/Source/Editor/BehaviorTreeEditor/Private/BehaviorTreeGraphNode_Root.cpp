// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "BehaviorTreeEditorPrivatePCH.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"

UBehaviorTreeGraphNode_Root::UBehaviorTreeGraphNode_Root(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UBehaviorTreeGraphNode_Root::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	// pick first available blackboard asset, hopefully something will be loaded...
	for (FObjectIterator It(UBlackboardData::StaticClass()); It; ++It)
	{
		UBlackboardData* TestOb = (UBlackboardData*)*It;
		if (!TestOb->HasAnyFlags(RF_ClassDefaultObject))
		{
			BlackboardAsset = TestOb;
			UpdateBlackboard();
			break;
		}
	}
}

void UBehaviorTreeGraphNode_Root::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UBehaviorTreeEditorTypes::PinCategory_SingleComposite, TEXT(""), NULL, false, false, TEXT("In"));
}

FText UBehaviorTreeGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("BehaviorTreeGraphNode", "Root", "ROOT");
}

FName UBehaviorTreeGraphNode_Root::GetNameIcon() const
{
	return FName("BTEditor.Graph.BTNode.Root.Icon");
}

FText UBehaviorTreeGraphNode_Root::GetTooltipText() const
{
	return UEdGraphNode::GetTooltipText();
}

void UBehaviorTreeGraphNode_Root::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UBehaviorTreeGraphNode_Root, BlackboardAsset))
	{
		UpdateBlackboard();
	}
}

void UBehaviorTreeGraphNode_Root::PostEditUndo()
{
	Super::PostEditUndo();
	UpdateBlackboard();
}

FString	UBehaviorTreeGraphNode_Root::GetDescription() const
{
	return *GetNameSafe(BlackboardAsset);
}

void UBehaviorTreeGraphNode_Root::UpdateBlackboard()
{
	UBehaviorTreeGraph* MyGraph = GetBehaviorTreeGraph();
	UBehaviorTree* BTAsset = Cast<UBehaviorTree>(MyGraph->GetOuter());
	if (BTAsset && BTAsset->BlackboardAsset != BlackboardAsset)
	{
		BTAsset->BlackboardAsset = BlackboardAsset;
		MyGraph->UpdateBlackboardChange();
	}
}
