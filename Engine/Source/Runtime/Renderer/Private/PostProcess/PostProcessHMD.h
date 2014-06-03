// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	PostProcessHMD.h: Post processing for HMD devices 
=============================================================================*/

#pragma once

#include "RenderingCompositionGraph.h"

/** The vertex data used to filter a texture. */
struct FDistortionVertex
{
	FVector2D Position;
	FVector2D TexR;
	FVector2D TexG;
	FVector2D TexB;
	FVector4  Color;
};

// derives from TRenderingCompositePassBase<InputCount, OutputCount>
// ePId_Input0: SceneColor
class FRCPassPostProcessHMD : public TRenderingCompositePassBase<1, 1>
{
public:
	// interface FRenderingCompositePass ---------
	virtual void Process(FRenderingCompositePassContext& Context);
	virtual void Release() { delete this; }
	FPooledRenderTargetDesc ComputeOutputDesc(EPassOutputId InPassOutputId) const;
};

