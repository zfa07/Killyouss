// %BANNER_BEGIN%
// ---------------------------------------------------------------------
// %COPYRIGHT_BEGIN%
//
// Copyright (c) 2017 Magic Leap, Inc. (COMPANY) All Rights Reserved.
// Magic Leap, Inc. Confidential and Proprietary
//
// NOTICE: All information contained herein is, and remains the property
// of COMPANY. The intellectual and technical concepts contained herein
// are proprietary to COMPANY and may be covered by U.S. and Foreign
// Patents, patents in process, and are protected by trade secret or
// copyright law. Dissemination of this information or reproduction of
// this material is strictly forbidden unless prior written permission is
// obtained from COMPANY. Access to the source code contained herein is
// hereby forbidden to anyone except current COMPANY employees, managers
// or contractors who have executed Confidentiality and Non-disclosure
// agreements explicitly covering such access.
//
// The copyright notice above does not evidence any actual or intended
// publication or disclosure of this source code, which includes
// information that is confidential and/or proprietary, and is a trade
// secret, of COMPANY. ANY REPRODUCTION, MODIFICATION, DISTRIBUTION,
// PUBLIC PERFORMANCE, OR PUBLIC DISPLAY OF OR THROUGH USE OF THIS
// SOURCE CODE WITHOUT THE EXPRESS WRITTEN CONSENT OF COMPANY IS
// STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE LAWS AND
// INTERNATIONAL TREATIES. THE RECEIPT OR POSSESSION OF THIS SOURCE
// CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS
// TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE,
// USE, OR SELL ANYTHING THAT IT MAY DESCRIBE, IN WHOLE OR IN PART.
//
// %COPYRIGHT_END%
// --------------------------------------------------------------------
// %BANNER_END%

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Tickable.h"
#include "MagicLeapIdentityTypes.h"
#include "MagicLeapIdentity.generated.h"

/**
 *  Class which provides functions to read and update the user's Magic Leap profile.
 */
UCLASS(ClassGroup = MagicLeap, BlueprintType)
class MAGICLEAPIDENTITY_API UMagicLeapIdentity : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UMagicLeapIdentity();
	virtual ~UMagicLeapIdentity();

	/**
	  Delegate for the result of available attributes for the user's Magic Leap profile.
	  @param ResultCode Error code when getting the available attributes.
	  @param AvailableAttributes List of attributes available for the user's Magic Leap profile.
	*/
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FAvailableIdentityAttributesDelegate, EMagicLeapIdentityError, ResultCode, const TArray<EMagicLeapIdentityAttribute>&, AvailableAttributes);

	/**
	  Delegate for the result of attribute values for the user's Magic Leap profile.
	  @param ResultCode Error code when getting the attribute values.
	  @param AttributeValue List of attribute values for the user's Magic Leap profile.
	*/
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FRequestIdentityAttributeValueDelegate, EMagicLeapIdentityError, ResultCode, const TArray<FMagicLeapIdentityAttribute>&, AttributeValue);

	/**
	  Delegate for the result of modifying the attribute values of a user's Magic Leap profile.
	  @param ResultCode Error code when modifying the attribute values.
	  @param AttributesUpdatedSuccessfully List of attributes whose values were successfully modified.
	*/
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FModifyIdentityAttributeValueDelegate, EMagicLeapIdentityError, ResultCode, const TArray<EMagicLeapIdentityAttribute>&, AttributesUpdatedSuccessfully);

	/**
	  Get the attributes available for the user's Magic Leap profile. Note that this does not request the values for these attribtues.
	  This function makes a blocking call to the cloud. You can alternatively use GetAllAvailableAttributesAsync() to request the attributes asynchronously.
	  @param AvailableAttributes Output parameter populated with the list of attributes available for the user's Magic Leap profile.
	  @return Error code when getting the list of available attributes.
	*/
	UFUNCTION(BlueprintCallable, Category = "Identity|MagicLeap")
	EMagicLeapIdentityError GetAllAvailableAttributes(TArray<EMagicLeapIdentityAttribute>& AvailableAttributes);

	/**
	  Asynchronous call to get the attributes available for the user's Magic Leap profile. Note that this does not request the values for these attribtues.
	  @param ResultDelegate Callback which reports the list of available attributes.
	  @return Error code when getting the list of available attributes.
	*/
	UFUNCTION(BlueprintCallable, Category = "Identity|MagicLeap")
	void GetAllAvailableAttributesAsync(const FAvailableIdentityAttributesDelegate& ResultDelegate);

	/**
	  Get the values for the attributes of the user's Magic Leap profile.
	  This function makes a blocking call to the cloud. You can alternatively use RequestAttributeValueAsync() to request the attribute values asynchronously.
	  @param RequestedAttributeList List of attributes to request the value for.
	  @param RequestedAttributeValues Output parameter populated with the list of attributes and their values.
	  @return Error code when getting the attribute values.
	*/
	UFUNCTION(BlueprintCallable, Category = "Identity|MagicLeap")
	EMagicLeapIdentityError RequestAttributeValue(const TArray<EMagicLeapIdentityAttribute>& RequestedAttributeList, TArray<FMagicLeapIdentityAttribute>& RequestedAttributeValues);

	/**
	  Asynchronous call to get the values for the attributes of the user's Magic Leap profile.
	  @param RequestedAttributeList List of attributes to request the value for.
	  @param ResultDelegate Callback which reports the list of attributes and their values.
	  @return Error code when getting the attribute values.
	*/
	UFUNCTION(BlueprintCallable, Category = "Identity|MagicLeap")
	EMagicLeapIdentityError RequestAttributeValueAsync(const TArray<EMagicLeapIdentityAttribute>& RequestedAttributeList, const FRequestIdentityAttributeValueDelegate& ResultDelegate);

	/**
	  Modify the values for the attributes of the user's Magic Leap profile.
	  This function makes a blocking call to the cloud. You can alternatively use ModifyAttributeValueAsync() to modify the attribute values asynchronously.
	  @param UpdatedAttributeValueList List of attributes and the values to update the cloud profile with.
	  @param AttributesUpdatedSuccessfully Output parameter populated with the list of attributes whose values were updated successfully.
	  @return Error code when modifying the attribute values.
	*/
	DEPRECATED(4.19, "This function is deprecated and will be removed in future releases.")
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ModifyAttributeValue_DEPRECATED"), Category = "Identity|MagicLeap")
	EMagicLeapIdentityError ModifyAttributeValue(const TArray<FMagicLeapIdentityAttribute>& UpdatedAttributeValueList, TArray<EMagicLeapIdentityAttribute>& AttributesUpdatedSuccessfully);

	/**
	  Asynchronous call to modify the values for the attributes of the user's Magic Leap profile.
	  @param UpdatedAttributeValueList List of attributes and the values to update the cloud profile with.
	  @param ResultDelegate Callback which reports the list of attributes whose values were successfully modified.
	  @return Error code when modifying the attribute values.
	*/
	DEPRECATED(4.19, "This function is deprecated and will be removed in future releases.")
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ModifyAttributeValueAsync_DEPRECATED"), Category = "Identity|MagicLeap")
	EMagicLeapIdentityError ModifyAttributeValueAsync(const TArray<FMagicLeapIdentityAttribute>& UpdatedAttributeValueList, const FModifyIdentityAttributeValueDelegate& ResultDelegate);

	/** UObjectBase interface */
	virtual UWorld* GetWorld() const override;

	/** FTickableObjectBase interface */
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	/** FTickableGameObject interface */
	virtual UWorld* GetTickableGameObjectWorld() const override;

private:
	class FIdentityImpl *Impl;
};
