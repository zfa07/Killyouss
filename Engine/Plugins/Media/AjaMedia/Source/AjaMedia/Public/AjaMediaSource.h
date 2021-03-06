// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TimeSynchronizableMediaSource.h"

#include "AjaMediaFinder.h"
#include "Misc/FrameRate.h"

#include "AjaMediaSource.generated.h"

/**
 * Available input color formats for Aja sources.
 */
UENUM()
enum class EAjaMediaColorFormat : uint8
{
	BGRA UMETA(DisplayName="RGBA"),
	UYVY UMETA(DisplayName="YUV 4:2:2"),
};

/**
 * Available timecode formats for Aja sources.
 */
UENUM()
enum class EAjaMediaTimecodeFormat : uint8
{
	None,
	LTC,
	VITC,
};

/**
 * Available number of audio channel supported by UE4 & Aja
 */
UENUM()
enum class EAjaMediaAudioChannel : uint8
{
	Channel6,
	Channel8,
};

/**
 * Media source for Aja streams.
 */
UCLASS(BlueprintType)
class AJAMEDIA_API UAjaMediaSource : public UTimeSynchronizableMediaSource
{
	GENERATED_BODY()

	/** Default constructor. */
	UAjaMediaSource();

public:
	/**
	 * The input name of the AJA source to be played".
	 * This combines the device ID, and the input.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AJA", AssetRegistrySearchable)
	FAjaMediaPort MediaPort;

	/** Source FrameRate(default = 30). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AJA")
	FFrameRate FrameRate;

	/** Use the time code embedded in the input stream. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AJA")
	EAjaMediaTimecodeFormat TimecodeFormat;

public:
	/**
	 * The capture of the Audio, Ancillary and/or video will be perform at the same time.
	 * This may decrease transfer performance but each the data will be sync in relation with each other.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Capture")
	bool bCaptureWithAutoCirculating;

	/**
	 * Capture Ancillary from the AJA source.
	 * It will decrease performance
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Capture", meta=(EditCondition="bCaptureWithAutoCirculating"))
	bool bCaptureAncillary1;

	/**
	 * Capture Ancillary from the AJA source.
	 * It will decrease performance
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Capture", meta=(EditCondition="bCaptureWithAutoCirculating"))
	bool bCaptureAncillary2;

	/**
	 * Capture Audio from the AJA source.
	 * It will decrease performance
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Capture", meta=(EditCondition="bCaptureWithAutoCirculating"))
	bool bCaptureAudio;

	/**
	 * Capture Video from the AJA source.
	 * It will decrease performance
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Capture")
	bool bCaptureVideo;

public:
	/** Maximum number of ancillary data frames to buffer. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category="Ancillary", meta=(EditCondition="IN_CPP", ClampMin="1", ClampMax="32"))
	int32 MaxNumAncillaryFrameBuffer;

public:
	/** Desired number of audio channel to capture. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Audio", meta=(EditCondition="bCaptureAudio"))
	EAjaMediaAudioChannel AudioChannel;

	/** Maximum number of audio frames to buffer. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category="Audio", meta=(EditCondition="bCaptureAudio", ClampMin="1", ClampMax="32"))
	int32 MaxNumAudioFrameBuffer;

public:
	/**
	 * Specifies if the video format is expected to be progressive or not.
	 * The hardware has no way of knowing if the incoming signal is progressive or interlaced (e.g., 525/29.97fps progressive versus 525/59.94fps interlaced)
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Video", meta=(EditCondition="bCaptureVideo"))
	bool bIsProgressivePicture;

	/** Desired color format of input video frames (default = BGRA). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Video", meta=(EditCondition="bCaptureVideo"))
	EAjaMediaColorFormat ColorFormat;

	/** Maximum number of video frames to buffer. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category="Video", meta=(EditCondition="bCaptureVideo", ClampMin="1", ClampMax="32"))
	int32 MaxNumVideoFrameBuffer;

public:
	/**
	 * Encode Timecode in the output
	 * Current value will be white. The format will be encoded in hh:mm::ss::ff. Each value, will be on a different line.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Debug", meta=(EditCondition="IN_CPP"))
	bool bEncodeTimecodeInTexel;

public:
	//~ IMediaOptions interface

	virtual bool GetMediaOption(const FName& Key, bool DefaultValue) const override;
	virtual int64 GetMediaOption(const FName& Key, int64 DefaultValue) const override;
	virtual bool HasMediaOption(const FName& Key) const override;

public:
	//~ UMediaSource interface

	virtual FString GetUrl() const override;
	virtual bool Validate() const override;

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
};
