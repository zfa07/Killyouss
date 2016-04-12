// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	IStereoLayers.h: Abstract interface for adding in stereoscopically projected
	layers on top of the world
=============================================================================*/

#pragma once

class IStereoLayers
{
public:

	enum ELayerType
	{
		WorldLocked,
		TorsoLocked,
		FaceLocked
	};

	/**
	 * Structure describing the visual appearance of a single stereo layer
	 */
	struct FLayerDesc
	{
		FTransform	Transform	= FTransform::Identity;									// View space transform
		FVector2D	QuadSize	= FVector2D(1.0f, 1.0f);								// Size of rendered quad
		FBox2D		UVRect		= FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));	// UVs of rendered quad
		int32		Priority	= 0;													// Render order priority, higher priority render on top of lower priority
		ELayerType	Type		= ELayerType::FaceLocked;								// Which space the quad is locked within
		UTexture2D*	Texture		= nullptr;												// Texture mapped to the quad
	};

	/**
	 * Creates a new layer from a given texture resource, which is projected on top of the world as a quad
	 *
	 * @param	InLayerDesc		A reference to the texture resource to be used on the quad
	 * @return	A unique identifier for the layer created
	 */
	virtual uint32 CreateLayer(const FLayerDesc& InLayerDesc) = 0;
	
	/**
	 * Destroys the specified layer, stopping it from rendering over the world
	 *
	 * @param	LayerId		The ID of layer to be destroyed
	 */
	virtual void DestroyLayer(uint32 LayerId) = 0;

	/**
	 * Set the a new layer description
	 *
	 * @param	LayerId		The ID of layer to be set the description
	 * @param	InLayerDesc	The new description to be set
	 */
	virtual void SetLayerDesc(uint32 LayerId, const FLayerDesc& InLayerDesc) = 0;

	/**
	 * Get the currently set layer description
	 *
	 * @param	LayerId			The ID of layer to be set the description
	 * @param	OutLayerDesc	The returned layer description
	 * @return	Whether the returned layer description is valid
	 */
	virtual bool GetLayerDesc(uint32 LayerId, FLayerDesc& OutLayerDesc) = 0;
};
