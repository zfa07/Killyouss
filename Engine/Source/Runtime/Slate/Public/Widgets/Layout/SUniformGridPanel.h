// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

/** A panel that evenly divides up available space between all of its children. */
class SLATE_API SUniformGridPanel : public SPanel
{
public:
	/** Stores the per-child info for this panel type */
	struct FSlot : public TSupportsOneChildMixin<SWidget, FSlot>, public TSupportsContentAlignmentMixin<FSlot>
	{
		FSlot( int32 InColumn, int32 InRow )
		: TSupportsContentAlignmentMixin<FSlot>(HAlign_Fill, VAlign_Fill)
		, Column( InColumn )
		, Row( InRow )
		{
		}

		int32 Column;
		int32 Row;
	};

	/**
	 * Used by declarative syntax to create a Slot in the specified Column, Row.
	 */
	static FSlot& Slot( int32 Column, int32 Row )
	{
		return *(new FSlot( Column, Row ));
	}

	SLATE_BEGIN_ARGS( SUniformGridPanel )
		: _SlotPadding( FMargin(0.0f) )
		, _MinDesiredSlotWidth( 0.0f )
		, _MinDesiredSlotHeight( 0.0f )
		{
			_Visibility = EVisibility::SelfHitTestInvisible;
		}

		/** Slot type supported by this panel */
		SLATE_SUPPORTS_SLOT(FSlot)
		
		/** Padding given to each slot */
		SLATE_ATTRIBUTE(FMargin, SlotPadding)

		/** The minimum desired width of the slots */
		SLATE_ATTRIBUTE(float, MinDesiredSlotWidth)

		/** The minimum desired height of the slots */
		SLATE_ATTRIBUTE(float, MinDesiredSlotHeight)

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs );

	// BEGIN SPanel INTERFACE	
	virtual void OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const override;
	virtual FVector2D ComputeDesiredSize() const override;
	virtual FChildren* GetChildren() override;
	// END SPanel INTERFACE

	/** See SlotPadding attribute */
	void SetSlotPadding(TAttribute<FMargin> InSlotPadding);

	/**
	 * Dynamically add a new slot to the UI at specified Column and Row.
	 *
	 * @return A reference to the newly-added slot
	 */
	FSlot& AddSlot( int32 Column, int32 Row );
	
	/**
	 * Removes a slot from this panel which contains the specified SWidget
	 *
	 * @param SlotWidget The widget to match when searching through the slots
	 * @returns The true if the slot was removed and false if no slot was found matching the widget
	 */
	bool RemoveSlot( const TSharedRef<SWidget>& SlotWidget );

	/** Removes all slots from the panel */
	void ClearChildren();

private:
	TPanelChildren<FSlot> Children;
	TAttribute<FMargin> SlotPadding;
	
	/** These values are recomputed and cached during compute desired size, as they may have changed since the previous frame. */
	mutable int32 NumColumns;
	mutable int32 NumRows;

	TAttribute<float> MinDesiredSlotWidth;
	TAttribute<float> MinDesiredSlotHeight;
};