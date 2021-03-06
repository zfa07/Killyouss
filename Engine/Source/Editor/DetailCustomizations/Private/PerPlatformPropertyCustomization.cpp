// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PerPlatformPropertyCustomization.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "SPerPlatformPropertiesWidget.h"
#include "ScopedTransaction.h"
#include "IPropertyUtilities.h"

template<typename PerPlatformType>
void FPerPlatformPropertyCustomization<PerPlatformType>::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	HeaderRow.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(CalcDesiredWidth(StructPropertyHandle))
	.MaxDesiredWidth(CalcDesiredWidth(StructPropertyHandle))
	[
		SNew(SPerPlatformPropertiesWidget)
		.OnGenerateWidget(this, &FPerPlatformPropertyCustomization<PerPlatformType>::GetWidget, StructPropertyHandle)
		.OnAddPlatform(this, &FPerPlatformPropertyCustomization<PerPlatformType>::AddPlatformOverride, StructPropertyHandle)
		.OnRemovePlatform(this, &FPerPlatformPropertyCustomization<PerPlatformType>::RemovePlatformOverride, StructPropertyHandle)
		.PlatformOverrideNames(this, &FPerPlatformPropertyCustomization<PerPlatformType>::GetPlatformOverrideNames, StructPropertyHandle)
	];
}

template<typename PerPlatformType>
TSharedRef<SWidget> FPerPlatformPropertyCustomization<PerPlatformType>::GetWidget(FName PlatformGroupName, TSharedRef<IPropertyHandle> StructPropertyHandle) const
{
	TSharedPtr<IPropertyHandle>	EditProperty;

	if (PlatformGroupName == NAME_None)
	{
		EditProperty = StructPropertyHandle->GetChildHandle(FName("Default"));
	}
	else
	{
		TSharedPtr<IPropertyHandle>	MapProperty = StructPropertyHandle->GetChildHandle(FName("PerPlatform"));
		if (MapProperty.IsValid())
		{
			uint32 NumChildren = 0;
			MapProperty->GetNumChildren(NumChildren);
			for (uint32 ChildIdx = 0; ChildIdx < NumChildren; ChildIdx++)
			{
				TSharedPtr<IPropertyHandle> ChildProperty = MapProperty->GetChildHandle(ChildIdx);
				if (ChildProperty.IsValid())
				{
					TSharedPtr<IPropertyHandle> KeyProperty = ChildProperty->GetKeyHandle();
					if (KeyProperty.IsValid())
					{
						FName KeyName;
						if(KeyProperty->GetValue(KeyName) == FPropertyAccess::Success && KeyName == PlatformGroupName)
						{
							EditProperty = ChildProperty;
							break;
						}
					}
				}
			}
		}
	}

	if (EditProperty.IsValid())
	{
		return EditProperty->CreatePropertyValueWidget(false);
	}
	else
	{
		return
			SNew(STextBlock)
			.Text(NSLOCTEXT("FPerPlatformPropertyCustomization", "GetWidget", "Could not find valid property"))
			.ColorAndOpacity(FLinearColor::Red);
	}
}

template<typename PerPlatformType>
float FPerPlatformPropertyCustomization<PerPlatformType>::CalcDesiredWidth(TSharedRef<IPropertyHandle> StructPropertyHandle)
{
	int32 NumOverrides = 0;
	TSharedPtr<IPropertyHandle>	MapProperty = StructPropertyHandle->GetChildHandle(FName("PerPlatform"));
	if (MapProperty.IsValid())
	{
		TArray<const void*> RawData;
		MapProperty->AccessRawData(RawData);
		for (const void* Data : RawData)
		{
			const TMap<FName, typename PerPlatformType::ValueType>* PerPlatformMap = (const TMap<FName, typename PerPlatformType::ValueType>*)(Data);
			NumOverrides = FMath::Max<int32>(PerPlatformMap->Num(), NumOverrides);
		}
	}
	return (float)(1 + NumOverrides) * 125.f;
}

template<typename PerPlatformType>
bool FPerPlatformPropertyCustomization<PerPlatformType>::AddPlatformOverride(FName PlatformGroupName, TSharedRef<IPropertyHandle> StructPropertyHandle)
{
	TSharedPtr<IPropertyHandle>	PerPlatformProperty = StructPropertyHandle->GetChildHandle(FName("PerPlatform"));
	TSharedPtr<IPropertyHandle>	DefaultProperty = StructPropertyHandle->GetChildHandle(FName("Default"));
	if (PerPlatformProperty.IsValid() && DefaultProperty.IsValid())
	{
		TSharedPtr<IPropertyHandleMap> MapProperty = PerPlatformProperty->AsMap();
		if (MapProperty.IsValid())
		{
			MapProperty->AddItem();
			uint32 NumChildren = 0;
			PerPlatformProperty->GetNumChildren(NumChildren);
			for (uint32 ChildIdx = 0; ChildIdx < NumChildren; ChildIdx++)
			{
				TSharedPtr<IPropertyHandle> ChildProperty = PerPlatformProperty->GetChildHandle(ChildIdx);
				if (ChildProperty.IsValid())
				{
					TSharedPtr<IPropertyHandle> KeyProperty = ChildProperty->GetKeyHandle();
					if (KeyProperty.IsValid())
					{
						FName KeyName;
						if (KeyProperty->GetValue(KeyName) == FPropertyAccess::Success && KeyName == NAME_None)
						{
							// Set Key
							KeyProperty->SetValue(PlatformGroupName);

							// Set Value
							typename PerPlatformType::ValueType DefaultValue;
							DefaultProperty->GetValue(DefaultValue);
							ChildProperty->SetValue(DefaultValue);

							if(PropertyUtilities.IsValid())
							{
								PropertyUtilities.Pin()->ForceRefresh();
							}

							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

template<typename PerPlatformType>
bool FPerPlatformPropertyCustomization<PerPlatformType>::RemovePlatformOverride(FName PlatformGroupName, TSharedRef<IPropertyHandle> StructPropertyHandle)
{
	TSharedPtr<IPropertyHandle>	PerPlatformProperty = StructPropertyHandle->GetChildHandle(FName("PerPlatform"));
	if (PerPlatformProperty.IsValid())
	{
		TSharedPtr<IPropertyHandleMap> MapProperty = PerPlatformProperty->AsMap();
		if (MapProperty.IsValid())
		{
			uint32 NumChildren = 0;
			PerPlatformProperty->GetNumChildren(NumChildren);
			for (uint32 ChildIdx = 0; ChildIdx < NumChildren; ChildIdx++)
			{
				TSharedPtr<IPropertyHandle> ChildProperty = PerPlatformProperty->GetChildHandle(ChildIdx);
				if (ChildProperty.IsValid())
				{
					TSharedPtr<IPropertyHandle> KeyProperty = ChildProperty->GetKeyHandle();
					if (KeyProperty.IsValid())
					{
						FName KeyName;
						if (KeyProperty->GetValue(KeyName) == FPropertyAccess::Success && KeyName == PlatformGroupName)
						{
							MapProperty->DeleteItem(ChildIdx);

							if(PropertyUtilities.IsValid())
							{
								PropertyUtilities.Pin()->ForceRefresh();
							}

							return true;
						}
					}
				}
			}
		}

		return true;
	}
	return false;
}

template<typename PerPlatformType>
TArray<FName> FPerPlatformPropertyCustomization<PerPlatformType>::GetPlatformOverrideNames(TSharedRef<IPropertyHandle> StructPropertyHandle) const
{
	TArray<FName> PlatformOverrideNames;

	TSharedPtr<IPropertyHandle>	MapProperty = StructPropertyHandle->GetChildHandle(FName("PerPlatform"));
	if (MapProperty.IsValid())
	{
		TArray<const void*> RawData;
		MapProperty->AccessRawData(RawData);
		for (const void* Data : RawData)
		{
			const TMap<FName, typename PerPlatformType::ValueType>* PerPlatformMap = (const TMap<FName, typename PerPlatformType::ValueType>*)(Data);
			check(PerPlatformMap);
			TArray<FName> KeyArray;
			PerPlatformMap->GenerateKeyArray(KeyArray);
			for (FName PlatformName : KeyArray)
			{
				PlatformOverrideNames.AddUnique(PlatformName);
			}
		}

	}
	return PlatformOverrideNames;
}

template<typename PerPlatformType>
TSharedRef<IPropertyTypeCustomization> FPerPlatformPropertyCustomization<PerPlatformType>::MakeInstance()
{
	return MakeShareable(new FPerPlatformPropertyCustomization<PerPlatformType>);
}

/* Only explicitly instantiate the types which are supported
*****************************************************************************/

template class FPerPlatformPropertyCustomization<FPerPlatformInt>;
template class FPerPlatformPropertyCustomization<FPerPlatformFloat>;

