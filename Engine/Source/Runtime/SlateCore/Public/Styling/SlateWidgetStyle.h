// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateWidgetStyle.generated.h"


/**
 * Base structure for widget styles.
 */
USTRUCT()
struct SLATECORE_API FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

public:

	/** Default constructor. */
	FSlateWidgetStyle( );

	/** Virtual Destructor. */
	virtual ~FSlateWidgetStyle() { }

public:

	/**
	 * Gets the brush resources associated with this style.
	 *
	 * This method must be implemented by inherited structures.
	 *
	 * @param OutBrushes The brush resources.
	 */
	virtual void GetResources( TArray<const FSlateBrush*> & OutBrushes ) const { }

	/**
	 * Gets the name of this style.
	 *
	 * This method must be implemented by inherited structures.
	 *
	 * @return Widget style name.
	 */
	virtual const FName GetTypeName() const
	{
		return FName( NAME_None );
	} 
};
