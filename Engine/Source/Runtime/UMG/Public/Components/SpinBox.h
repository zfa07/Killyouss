// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SpinBox.generated.h"

/**
 * A numerical entry box that allows for direct entry of the number or allows the user to click and slide the number.
 */
UCLASS()
class UMG_API USpinBox : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpinBoxValueChangedEvent, float, InValue);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpinBoxValueCommittedEvent, float, InValue, ETextCommit::Type, CommitMethod);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpinBoxBeginSliderMovement);

public:

	/** Value stored in this spin box */
	UPROPERTY(EditDefaultsOnly, Category=Content)
	float Value;

	/** A bindable delegate to allow logic to drive the value of the widget */
	UPROPERTY()
	FGetFloat ValueDelegate;

public:
	/** The Style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style", meta=( DisplayName="Style" ))
	FSpinBoxStyle WidgetStyle;

	UPROPERTY()
	USlateWidgetStyleAsset* Style_DEPRECATED;

	/** The amount by which to change the spin box value as the slider moves. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Slider")
	float Delta;

	/** The exponent by which to increase the delta as the mouse moves. 1 is constant (never increases the delta). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Slider")
	float SliderExponent;
	
	/** Font color and opacity (overrides style) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display")
	FSlateFontInfo Font;

	/** The minimum width of the spin box */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display", AdvancedDisplay, DisplayName = "Minimum Desired Width")
	float MinDesiredWidth;

	/** Whether to remove the keyboard focus from the spin box when the value is committed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", AdvancedDisplay)
	bool ClearKeyboardFocusOnCommit;

	/** Whether to select the text in the spin box when the value is committed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", AdvancedDisplay)
	bool SelectAllTextOnCommit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Style")
	FSlateColor ForegroundColor;

public:
	/** Called when the value is changed interactively by the user */
	UPROPERTY(BlueprintAssignable, Category="SpinBox|Events")
	FOnSpinBoxValueChangedEvent OnValueChanged;

	/** Called when the value is committed. Occurs when the user presses Enter or the text box loses focus. */
	UPROPERTY(BlueprintAssignable, Category="SpinBox|Events")
	FOnSpinBoxValueCommittedEvent OnValueCommitted;

	/** Called right before the slider begins to move */
	UPROPERTY(BlueprintAssignable, Category="SpinBox|Events")
	FOnSpinBoxBeginSliderMovement OnBeginSliderMovement;

	/** Called right after the slider handle is released by the user */
	UPROPERTY(BlueprintAssignable, Category="SpinBox|Events")
	FOnSpinBoxValueChangedEvent OnEndSliderMovement;

public:

	/** Get the current value of the spin box. */
	UFUNCTION(BlueprintCallable, Category="Behavior")
	float GetValue() const;

	/** Set the value of the spin box. */
	UFUNCTION(BlueprintCallable, Category="Behavior")
	void SetValue(float NewValue);

public:

	/** Get the current minimum value that can be manually set in the spin box. */
	UFUNCTION(BlueprintCallable, Category="Behavior")
	float GetMinValue() const;

	/** Set the minimum value that can be manually set in the spin box. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void SetMinValue(float NewValue);

	/** Clear the minimum value that can be manually set in the spin box. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void ClearMinValue();

	/** Get the current maximum value that can be manually set in the spin box. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	float GetMaxValue() const;

	/** Set the maximum value that can be manually set in the spin box. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void SetMaxValue(float NewValue);

	/** Clear the maximum value that can be manually set in the spin box. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void ClearMaxValue();

	/** Get the current minimum value that can be specified using the slider. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	float GetMinSliderValue() const;

	/** Set the minimum value that can be specified using the slider. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void SetMinSliderValue(float NewValue);

	/** Clear the minimum value that can be specified using the slider. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void ClearMinSliderValue();

	/** Get the current maximum value that can be specified using the slider. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	float GetMaxSliderValue() const;

	/** Set the maximum value that can be specified using the slider. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void SetMaxSliderValue(float NewValue);

	/** Clear the maximum value that can be specified using the slider. */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void ClearMaxSliderValue();

	/**  */
	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void SetForegroundColor(FSlateColor InForegroundColor);

public:

	// UWidget interface
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UVisual interface

	// Begin UObject interface
	virtual void PostLoad() override;
	// End of UObject interface

#if WITH_EDITOR
	virtual const FSlateBrush* GetEditorIcon() override;
	virtual const FText GetPaletteCategory() override;
#endif
	// End of UWidget interface

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget

	void HandleOnValueChanged(float InValue);
	void HandleOnValueCommitted(float InValue, ETextCommit::Type CommitMethod);
	void HandleOnBeginSliderMovement();
	void HandleOnEndSliderMovement(float InValue);

protected:
	/** Whether the optional MinValue attribute of the widget is set */
	UPROPERTY()
	uint32 bOverride_MinValue : 1;

	/** Whether the optional MaxValue attribute of the widget is set */
	UPROPERTY()
	uint32 bOverride_MaxValue : 1;

	/** Whether the optional MinSliderValue attribute of the widget is set */
	UPROPERTY()
	uint32 bOverride_MinSliderValue : 1;

	/** Whether the optional MaxSliderValue attribute of the widget is set */
	UPROPERTY()
	uint32 bOverride_MaxSliderValue : 1;

	/** The minimum allowable value that can be manually entered into the spin box */
	UPROPERTY(EditDefaultsOnly, Category = Content, DisplayName = "Minimum Value", meta = (editcondition = "bOverride_MinValue"))
	float MinValue;

	/** The maximum allowable value that can be manually entered into the spin box */
	UPROPERTY(EditDefaultsOnly, Category = Content, DisplayName = "Maximum Value", meta = (editcondition = "bOverride_MaxValue"))
	float MaxValue;

	/** The minimum allowable value that can be specified using the slider */
	UPROPERTY(EditDefaultsOnly, Category = Content, DisplayName = "Minimum Slider Value", meta = (editcondition = "bOverride_MinSliderValue"))
	float MinSliderValue;

	/** The maximum allowable value that can be specified using the slider */
	UPROPERTY(EditDefaultsOnly, Category = Content, DisplayName = "Maximum Slider Value", meta = (editcondition = "bOverride_MaxSliderValue"))
	float MaxSliderValue;

protected:
	TSharedPtr<SSpinBox<float>> MySpinBox;
};