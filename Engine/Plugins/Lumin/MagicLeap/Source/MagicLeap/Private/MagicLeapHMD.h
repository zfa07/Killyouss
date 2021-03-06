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

#include "HeadMountedDisplayBase.h"
#include "Runtime/Launch/Resources/Version.h"
#include "IMagicLeapPlugin.h"
#include "IMagicLeapHMD.h"
#include "Misc/ScopeLock.h"

#include "XRRenderTargetManager.h"
#include "AppFramework.h"
#include "SceneViewExtension.h"
#include "MagicLeapMath.h"
#include "MagicLeapCustomPresent.h"
#include "MagicLeapHMDFunctionLibrary.h"
#include "LuminRuntimeSettings.h"
#include "MagicLeapPluginUtil.h" // for ML_INCLUDES_START/END

#if WITH_MLSDK
ML_INCLUDES_START
#include <ml_head_tracking.h>
ML_INCLUDES_END
#endif //WITH_MLSDK

/**
  * MagicLeap Head Mounted Display
  */
class MAGICLEAP_API FMagicLeapHMD : public IMagicLeapHMD, public FHeadMountedDisplayBase, public FXRRenderTargetManager, public TSharedFromThis<FMagicLeapHMD, ESPMode::ThreadSafe>
{
public:

	static const FName SystemName;

	/** IXRTrackingSystem interface */
	virtual bool OnStartGameFrame(FWorldContext& WorldContext) override;
	virtual bool OnEndGameFrame(FWorldContext& WorldContext) override;
	virtual void OnBeginRendering_GameThread() override;
	virtual void OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily) override;
	virtual class IHeadMountedDisplay* GetHMDDevice() override { return this; }
	virtual class TSharedPtr< class IStereoRendering, ESPMode::ThreadSafe > GetStereoRenderingDevice() override { return AsShared(); }
	virtual class TSharedPtr< class IXRCamera, ESPMode::ThreadSafe > GetXRCamera(int32 DeviceId) override;
	virtual FName GetSystemName() const override;

	virtual bool DoesSupportPositionalTracking() const override;
	virtual bool HasValidTrackingPosition() override;
	virtual bool GetTrackingSensorProperties(int32 DeviceId, FQuat& OutOrientation, FVector& OutPosition, FXRSensorProperties& OutSensorProperties) override;
	virtual bool EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type /*= EXRTrackedDeviceType::Any*/) override;
	virtual bool GetCurrentPose(int32 DeviceId, FQuat& OutOrientation, FVector& OutPosition) override;
	virtual bool GetRelativeEyePose(int32 DeviceId, EStereoscopicPass Eye, FQuat& OutOrientation, FVector& OutPosition) override;

	virtual bool IsHeadTrackingAllowed() const override;

	virtual void ResetOrientationAndPosition(float yaw = 0.f) override;
	virtual void ResetOrientation(float Yaw = 0.f) override;
	virtual void ResetPosition() override;

	virtual void OnBeginPlay(FWorldContext& InWorldContext) override;
	virtual void OnEndPlay(FWorldContext& InWorldContext) override;

	void SetBasePosition(const FVector& InBasePosition);
	FVector GetBasePosition() const;

	virtual void SetBaseRotation(const FRotator& BaseRot) override;
	virtual FRotator GetBaseRotation() const override;

	virtual void SetBaseOrientation(const FQuat& BaseOrient) override;
	virtual FQuat GetBaseOrientation() const override;

	float GetWorldToMetersScale() const override;

	/** IHeadMountedDisplay interface */
	virtual bool IsHMDConnected() override;
	virtual bool IsHMDEnabled() const override;
	virtual void EnableHMD(bool allow = true) override;
	virtual bool GetHMDMonitorInfo(MonitorInfo&) override;
	virtual void SetInterpupillaryDistance(float NewInterpupillaryDistance) override;
	virtual float GetInterpupillaryDistance() const override;

	virtual bool IsChromaAbCorrectionEnabled() const override;

	virtual void GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const override;
	virtual void UpdateScreenSettings(const FViewport* InViewport) override {}


	/** IStereoRendering interface */
	virtual bool IsStereoEnabled() const override
	{
		return bStereoEnabled && bHmdEnabled;
	}

	virtual bool EnableStereo(bool bStereo = true) override;
	virtual void AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
	virtual void InitCanvasFromView(FSceneView* InView, UCanvas* Canvas) override;
	virtual void GetEyeRenderParams_RenderThread(const FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const override;
	virtual void CalculateRenderTargetSize(const class FViewport& Viewport, uint32& InOutSizeX, uint32& InOutSizeY) override;
	virtual bool NeedReAllocateViewportRenderTarget(const FViewport& Viewport) override;
	virtual bool AllocateRenderTargetTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 Flags, uint32 TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples = 1) override;
	//virtual FRHICustomPresent* GetCustomPresent() override;
	virtual IStereoRenderTargetManager* GetRenderTargetManager() override { return this; }

	virtual FMatrix GetStereoProjectionMatrix(const EStereoscopicPass StereoPassType) const override;

	virtual void GetOrthoProjection(int32 RTWidth, int32 RTHeight, float OrthoDistance, FMatrix OrthoProjection[2]) const override;
	virtual void SetClippingPlanes(float NCP, float FCP) override;
	virtual void RenderTexture_RenderThread(class FRHICommandListImmediate& RHICmdList, class FRHITexture2D* BackBuffer, class FRHITexture2D* SrcTexture, FVector2D WindowSize) const override;

	virtual bool GetHMDDistortionEnabled(EShadingPath ShadingPath) const override
	{
		return false;
	}

	// FXRRenderTargetManager interface
	virtual void UpdateViewportRHIBridge(bool bUseSeparateRenderTarget, const class FViewport& Viewport, FRHIViewport* const ViewportRHI) override;
	virtual bool ShouldUseSeparateRenderTarget() const override
	{
		check(IsInGameThread());
		return IsStereoEnabled();
	}

public:
	/** Constructor */
	FMagicLeapHMD(IMagicLeapPlugin* MagicLeapPlugin, bool bEnableVDZI = false);

	/** Destructor */
	virtual ~FMagicLeapHMD();

	/** FMagicLeapHMDBase interface */
	virtual void RegisterMagicLeapInputDevice(IMagicLeapInputDevice* InputDevice) override;
	virtual void UnregisterMagicLeapInputDevice(IMagicLeapInputDevice* InputDevice) override;
	virtual bool IsInitialized() const override;
	bool IsDeviceInitialized() const { return (bDeviceInitialized != 0) ? true : false; }

	bool GetHeadTrackingState(FHeadTrackingState& State) const;
	void UpdateNearClippingPlane();
	FMagicLeapCustomPresent* GetActiveCustomPresent(const bool bRequireDeviceIsInitialized = true) const;
	void ShutdownRendering();

	/** Get the windowed mirror mode.  @todo sensoryware: thread safe flags */
	int32 GetWindowMirrorMode() const { return WindowMirrorMode; }

	/** Enables, or disables, local input. Returns the previous value of ignore input. */
	bool SetIgnoreInput(bool Ignore);


	/** Utility class to scope guard enabling and disabling game viewport client input processing.
	* On creation it will enable the input processing, and on exit it will restore it to its
	* previous state. Usage is:
	*
	* { FMagicLeapHMD::EnableInput EnableInput; PostSomeInputToMessageHandlers(); }
	*/
	struct EnableInput
	{
#if WITH_EDITOR
		inline EnableInput()
		{
			SavedIgnoreInput = FMagicLeapHMD::GetHMD()->SetIgnoreInput(false);
		}
		inline ~EnableInput()
		{
			FMagicLeapHMD::GetHMD()->SetIgnoreInput(SavedIgnoreInput);
		}

	private:
		bool SavedIgnoreInput;
#endif
	};

public:

	uint32 GetViewportCount() const { return AppFramework.IsInitialized() ? AppFramework.GetViewportCount() : 0; }
	FTrackingFrame* GetCurrentFrame() const;
	FTrackingFrame* GetOldFrame() const;


	// HACK: This is a hack in order to pass variables from game frame to render frame
	// This should be removed once graphics provides vergence based focus distance
	void InitializeRenderFrameFromGameFrame();

	// HACK: This is a hack in order to use projection matrices from last render frame
	// This should be removed once unreal can use separate projection matrices for update and render
	void InitializeOldFrameFromRenderFrame();

	const FAppFramework& GetAppFrameworkConst() const;
	FAppFramework& GetAppFramework();
	void SetFocusActor(const AActor* InFocusActor);

	bool IsPerceptionEnabled() const;

	int32 WindowMirrorMode; // how to mirror the display contents to the desktop window: 0 - no mirroring, 1 - single eye, 2 - stereo pair
	uint32 DebugViewportWidth;
	uint32 DebugViewportHeight;
#if WITH_MLSDK
	MLHandle GraphicsClient;
#endif //WITH_MLSDK

private:

	// The FMagicLeapPlugin class functions as a factory for FMagicLeapHMD.
	// Therefore, it needs some control over initalization & termination.
	friend class FMagicLeapPlugin;

	/**
	* Starts up the SensoryWare API
	*/
	void Startup();

	/**
	* Shuts down the SensoryWare API
	*/
	void Shutdown();

	void EnableDeviceFeatures();
	void DisableDeviceFeatures();

	void InitDevice();
	void InitDevice_RenderThread();
	void ReleaseDevice();
	void ReleaseDevice_RenderThread();

	void LoadFromIni();
	void SaveToIni();

	void EnableLuminProfile();
	void RestoreBaseProfile();

	void EnableInputDevices();
	void DisableInputDevices();

	void EnablePerception();
	void DisablePerception();

	void EnableHeadTracking();
	void DisableHeadTracking();

	class FSceneViewport* FindSceneViewport();

	void InitializeClipExtents_RenderThread();
	void RefreshTrackingFrame();

#if WITH_MLSDK
	EHeadTrackingError MLToUnrealHeadTrackingError(MLHeadTrackingError error) const;
	EHeadTrackingMode MLToUnrealHeadTrackingMode(MLHeadTrackingMode mode) const;
#endif //WITH_MLSDK

#if !PLATFORM_LUMIN
	void DisplayWarningIfVDZINotEnabled();
#endif

#if PLATFORM_LUMIN
	/** Sets a frame timing hint, which tells the device what your target framerate is, to aid in prediction and jitter */
	void SetFrameTimingHint(ELuminFrameTimingHint InFrameTimingHint);
#endif //PLATFORM_LUMIN

private:
	int32 bDeviceInitialized; // RW on render thread, R on game thread
	int32 bDeviceWasJustInitialized; // RW on render thread, RW on game thread
	FAppFramework AppFramework;
	bool bHmdEnabled;
	bool bStereoEnabled;
#if !PLATFORM_LUMIN
	bool bStereoDesired;
#endif
	bool bHmdPosTracking;
	mutable bool bHaveVisionTracking;
	float IPD;
	FRotator DeltaControlRotation;
#if WITH_MLSDK
	MLHandle HeadTracker;
	MLHeadTrackingStaticData HeadTrackerData;
#endif //WITH_MLSDK
	IRendererModule* RendererModule;
	IMagicLeapPlugin* MagicLeapPlugin;
	float IdealScreenPercentage;
	bool bIsPlaying;
	bool bIsPerceptionEnabled;
	bool bIsVDZIEnabled;
	bool bVDZIWarningDisplayed;

	/** Current hint to the Lumin system about what our target framerate should be */
	ELuminFrameTimingHint CurrentFrameTimingHint;

#if PLATFORM_WINDOWS
	TRefCountPtr<FMagicLeapCustomPresentD3D11> CustomPresentD3D11;
#endif // PLATFORM_WINDOWS
#if PLATFORM_MAC
	TRefCountPtr<FMagicLeapCustomPresentMetal> CustomPresentMetal;
#endif // PLATFORM_MAC
#if PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_LUMIN
	TRefCountPtr<FMagicLeapCustomPresentOpenGL> CustomPresentOpenGL;
#endif // PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_LUMIN
#if PLATFORM_LUMIN
	TRefCountPtr<FMagicLeapCustomPresentVulkan> CustomPresentVulkan;
#endif // PLATFORM_LUMIN

public:
	FTrackingFrame GameTrackingFrame;
	FTrackingFrame RenderTrackingFrame;
	FTrackingFrame RHITrackingFrame;
	FTrackingFrame OldTrackingFrame;

private:
	TSet<IMagicLeapInputDevice*> InputDevices;
	TWeakObjectPtr<const AActor> FocusActor;

	struct SavedProfileState
	{
		bool bSaved;
		bool bCPUThrottleEnabled;
		TMap<FString, FString> CVarState;

		SavedProfileState()
			: bSaved(false)
			, bCPUThrottleEnabled(false)
		{
		}
	};
	SavedProfileState BaseProfileState;

	FHeadTrackingState HeadTrackingState;
	bool bHeadTrackingStateAvailable;

#if WITH_EDITOR
	/** The world we are playing. This is valid only within the span of BeginPlay & EndPlay. */
	UWorld* World;

	/** Indicator for needing to disable input at start of game. */
	bool DisableInputForBeginPlay;

	/** Get the game viewport client for the currently playing world. For PIE this is wherever
	* the current world is playing, i.e. rendering and handling input, in.
	*/
	UGameViewportClient* GetGameViewportClient();

	/** Utility to get the MagicLeap specific HMD plugin instance. */
	static FMagicLeapHMD* GetHMD();
#endif
};

//DEFINE_LOG_CATEGORY_STATIC(LogHMD, Log, All);
