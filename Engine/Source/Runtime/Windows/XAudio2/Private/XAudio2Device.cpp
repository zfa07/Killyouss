// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	XeAudioDevice.cpp: Unreal XAudio2 Audio interface object.

	Unreal is RHS with Y and Z swapped (or technically LHS with flipped axis)

=============================================================================*/

/*------------------------------------------------------------------------------------
	Audio includes.
------------------------------------------------------------------------------------*/

#include "XAudio2Device.h"
#include "AudioEffect.h"
#include "OpusAudioInfo.h"
#include "VorbisAudioInfo.h"
#include "XAudio2Effects.h"
#include "Engine.h"
#include "AllowWindowsPlatformTypes.h"
	#include <xapobase.h>
	#include <xapofx.h>
	#include <xaudio2fx.h>
#include "HideWindowsPlatformTypes.h"
#include "TargetPlatform.h"
#include "XAudio2Support.h"

DEFINE_LOG_CATEGORY(LogXAudio2);

class FXAudio2DeviceModule : public IAudioDeviceModule
{
public:

	/** Creates a new instance of the audio device implemented by the module. */
	virtual FAudioDevice* CreateAudioDevice() override
	{
		return new FXAudio2Device;
	}
};

IMPLEMENT_MODULE(FXAudio2DeviceModule, XAudio2);

/*------------------------------------------------------------------------------------
	Static variables from the early init
------------------------------------------------------------------------------------*/

// The number of speakers producing sound (stereo or 5.1)
int32 FXAudioDeviceProperties::NumSpeakers						= 0;
IXAudio2* FXAudioDeviceProperties::XAudio2						= NULL;
const float* FXAudioDeviceProperties::OutputMixMatrix			= NULL;
IXAudio2MasteringVoice* FXAudioDeviceProperties::MasteringVoice	= NULL;
#if XAUDIO_SUPPORTS_DEVICE_DETAILS
XAUDIO2_DEVICE_DETAILS FXAudioDeviceProperties::DeviceDetails	= { 0 };
#endif	//XAUDIO_SUPPORTS_DEVICE_DETAILS

/*------------------------------------------------------------------------------------
	FAudioDevice Interface.
------------------------------------------------------------------------------------*/

#define DEBUG_XAUDIO2 0

FSpatializationHelper FXAudio2Device::SpatializationHelper;

bool FXAudio2Device::InitializeHardware()
{
	if (IsRunningDedicatedServer())
	{
		return false;
	}

	// Load ogg and vorbis dlls if they haven't been loaded yet
	LoadVorbisLibraries();

	UINT32 SampleRate = 0;

#if PLATFORM_WINDOWS
	bComInitialized = FWindowsPlatformMisc::CoInitialize();
#if PLATFORM_64BITS
	// Work around the fact the x64 version of XAudio2_7.dll does not properly ref count
	// by forcing it to be always loaded
	LoadLibraryA( "XAudio2_7.dll" );
#endif	//PLATFORM_64BITS
#endif	//PLATFORM_WINDOWS

#if DEBUG_XAUDIO2
	uint32 Flags = XAUDIO2_DEBUG_ENGINE;
#else
	uint32 Flags = 0;
#endif
	if( XAudio2Create( &FXAudioDeviceProperties::XAudio2, Flags, AUDIO_HWTHREAD ) != S_OK )
	{
		UE_LOG(LogInit, Log, TEXT( "Failed to create XAudio2 interface" ) );
		return( false );
	}

#if XAUDIO_SUPPORTS_DEVICE_DETAILS
	UINT32 DeviceCount = 0;
	ValidateAPICall(TEXT("GetDeviceCount"),
		FXAudioDeviceProperties::XAudio2->GetDeviceCount( &DeviceCount ));
	if( DeviceCount < 1 )
	{
		UE_LOG(LogInit, Log, TEXT( "No audio devices found!" ) );
		FXAudioDeviceProperties::XAudio2 = NULL;
		return( false );		
	}

	// Get the details of the default device 0
	if( !ValidateAPICall(TEXT("GetDeviceDetails"),
			FXAudioDeviceProperties::XAudio2->GetDeviceDetails( 0, &FXAudioDeviceProperties::DeviceDetails )))
	{
		UE_LOG(LogInit, Log, TEXT( "Failed to get DeviceDetails for XAudio2" ) );
		FXAudioDeviceProperties::XAudio2 = NULL;
		return( false );
	}

#if DEBUG_XAUDIO2
	XAUDIO2_DEBUG_CONFIGURATION DebugConfig = {0};
	DebugConfig.TraceMask = XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_DETAIL;
	DebugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
	FXAudioDeviceProperties::XAudio2->SetDebugConfiguration(&DebugConfig);
#endif

	FXAudioDeviceProperties::NumSpeakers = UE4_XAUDIO2_NUMCHANNELS;
	SampleRate = FXAudioDeviceProperties::DeviceDetails.OutputFormat.Format.nSamplesPerSec;

	// Clamp the output frequency to the limits of the reverb XAPO
	if( SampleRate > XAUDIO2FX_REVERB_MAX_FRAMERATE )
	{
		SampleRate = XAUDIO2FX_REVERB_MAX_FRAMERATE;
		FXAudioDeviceProperties::DeviceDetails.OutputFormat.Format.nSamplesPerSec = SampleRate;
	}

	UE_LOG(LogInit, Log, TEXT( "XAudio2 using '%s' : %d channels at %g kHz using %d bits per sample (channel mask 0x%x)" ), 
		FXAudioDeviceProperties::DeviceDetails.DisplayName,
		FXAudioDeviceProperties::NumSpeakers, 
		( float )SampleRate / 1000.0f, 
		FXAudioDeviceProperties::DeviceDetails.OutputFormat.Format.wBitsPerSample,
		(uint32)UE4_XAUDIO2_CHANNELMASK );

	if( !GetOutputMatrix( UE4_XAUDIO2_CHANNELMASK, FXAudioDeviceProperties::NumSpeakers ) )
	{
		UE_LOG(LogInit, Log, TEXT( "Unsupported speaker configuration for this number of channels" ) );
		FXAudioDeviceProperties::XAudio2 = NULL;
		return( false );
	}

	// Create the final output voice with either 2 or 6 channels
	if (!ValidateAPICall(TEXT("CreateMasteringVoice"), 
			FXAudioDeviceProperties::XAudio2->CreateMasteringVoice(&FXAudioDeviceProperties::MasteringVoice, FXAudioDeviceProperties::NumSpeakers, SampleRate, 0, 0, NULL)))
	{
		UE_LOG(LogInit, Warning, TEXT( "Failed to create the mastering voice for XAudio2" ) );
		FXAudioDeviceProperties::XAudio2 = NULL;
		return( false );
	}
#else	//XAUDIO_SUPPORTS_DEVICE_DETAILS
	// Create the final output voice
	if (!ValidateAPICall(TEXT("CreateMasteringVoice"),
			FXAudioDeviceProperties::XAudio2->CreateMasteringVoice(&FXAudioDeviceProperties::MasteringVoice, UE4_XAUDIO2_NUMCHANNELS, UE4_XAUDIO2_SAMPLERATE, 0, 0, NULL )))
	{
		UE_LOG(LogInit, Warning, TEXT( "Failed to create the mastering voice for XAudio2" ) );
		FXAudioDeviceProperties::XAudio2 = NULL;
		return false;
	}
#endif	//XAUDIO_SUPPORTS_DEVICE_DETAILS

	SpatializationHelper.Init();

	// Initialize permanent memory stack for initial & always loaded sound allocations.
	if( CommonAudioPoolSize )
	{
		UE_LOG(LogAudio, Log, TEXT( "Allocating %g MByte for always resident audio data" ), CommonAudioPoolSize / ( 1024.0f * 1024.0f ) );
		CommonAudioPoolFreeBytes = CommonAudioPoolSize;
		CommonAudioPool = ( uint8* )FMemory::Malloc( CommonAudioPoolSize );
	}
	else
	{
		UE_LOG(LogAudio, Log, TEXT( "CommonAudioPoolSize is set to 0 - disabling persistent pool for audio data" ) );
		CommonAudioPoolFreeBytes = 0;
	}

	return true;
}

void FXAudio2Device::TeardownHardware()
{
	// close hardware interfaces
	if( FXAudioDeviceProperties::MasteringVoice )
	{
		FXAudioDeviceProperties::MasteringVoice->DestroyVoice();
		FXAudioDeviceProperties::MasteringVoice = NULL;
	}

	if( FXAudioDeviceProperties::XAudio2 )
	{
		// Force the hardware to release all references
		FXAudioDeviceProperties::XAudio2->Release();
		FXAudioDeviceProperties::XAudio2 = NULL;
	}

#if PLATFORM_WINDOWS
	if (bComInitialized)
	{
		FWindowsPlatformMisc::CoUninitialize();
	}
#endif
}

void FXAudio2Device::UpdateHardware()
{
	if (Listeners.Num() > 0)
	{
		// Caches the matrix used to transform a sounds position into local space so we can just look
		// at the Y component after normalization to determine spatialization.
		const FVector Up = Listeners[0].GetUp();
		const FVector Right = Listeners[0].GetFront();
		InverseTransform = FMatrix(Up, Right, Up ^ Right, Listeners[0].Transform.GetTranslation()).InverseFast();
	}
}

FAudioEffectsManager* FXAudio2Device::CreateEffectsManager()
{
	// Create the effects subsystem (reverb, EQ, etc.)
	return new FXAudio2EffectsManager(this);
}

FSoundSource* FXAudio2Device::CreateSoundSource()
{
	// create source source object
	return new FXAudio2SoundSource(this);
}

bool FXAudio2Device::HasCompressedAudioInfoClass(USoundWave* SoundWave)
{
#if WITH_OGGVORBIS
	return true;
#else
	return false;
#endif
}

class ICompressedAudioInfo* FXAudio2Device::CreateCompressedAudioInfo(USoundWave* SoundWave)
{
	if (SoundWave->IsStreaming())
	{
		return new FOpusAudioInfo();
	}

#if WITH_OGGVORBIS
	if (SoundWave->CompressionName.IsNone() || SoundWave->CompressionName == TEXT("OGG"))
	{

		return new FVorbisAudioInfo();
	}
	else
	{
		return NULL;
	}
#else
	return NULL;
#endif
	
}

/**  
 * Check for errors and output a human readable string 
 */
bool FXAudio2Device::ValidateAPICall( const TCHAR* Function, int32 ErrorCode )
{
	if( ErrorCode != S_OK )
	{
		switch( ErrorCode )
		{
		case XAUDIO2_E_INVALID_CALL:
			UE_LOG(LogAudio, Warning, TEXT( "%s error: Invalid Call" ), Function );
			break;

		case XAUDIO2_E_XMA_DECODER_ERROR:
			UE_LOG(LogAudio, Warning, TEXT( "%s error: XMA Decoder Error" ), Function );
			break;

		case XAUDIO2_E_XAPO_CREATION_FAILED:
			UE_LOG(LogAudio, Warning, TEXT( "%s error: XAPO Creation Failed" ), Function );
			break;

		case XAUDIO2_E_DEVICE_INVALIDATED:
			UE_LOG(LogAudio, Warning, TEXT( "%s error: Device Invalidated" ), Function );
			break;

		default:
			UE_LOG(LogAudio, Warning, TEXT( "%s error: Unhandled error code %d" ), Function, ErrorCode );
			break;
		};

		return( false );
	}

	return( true );
}




/** 
 * Derives the output matrix to use based on the channel mask and the number of channels
 */
const float OutputMatrixMono[SPEAKER_COUNT] = 
{ 
	0.7f, 0.7f, 0.5f, 0.0f, 0.5f, 0.5f	
};

const float OutputMatrix2dot0[SPEAKER_COUNT * 2] = 
{ 
	1.0f, 0.0f, 0.7f, 0.0f, 1.25f, 0.0f, // FL 
	0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 1.25f  // FR
};

//	const float OutputMatrixDownMix[SPEAKER_COUNT * 2] = 
//	{ 
//		1.0f, 0.0f, 0.7f, 0.0f, 0.87f, 0.49f,  
//		0.0f, 1.0f, 0.7f, 0.0f, 0.49f, 0.87f 
//	};

const float OutputMatrix2dot1[SPEAKER_COUNT * 3] = 
{ 
	// Same as stereo, but also passing in LFE signal
	1.0f, 0.0f, 0.7f, 0.0f, 1.25f, 0.0f, // FL
	0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 1.25f, // FR
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // LFE
};

const float OutputMatrix4dot0s[SPEAKER_COUNT * 4] = 
{ 
	// Combine both rear channels to make a rear center channel
	1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // FC
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f  // RC
};

const float OutputMatrix4dot0[SPEAKER_COUNT * 4] = 
{ 
	// Split the center channel to the front two speakers
	1.0f, 0.0f, 0.7f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // RL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // RR
};

const float OutputMatrix4dot1[SPEAKER_COUNT * 5] = 
{ 
	// Split the center channel to the front two speakers
	1.0f, 0.0f, 0.7f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.7f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // LFE
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // RL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // RR
};

const float OutputMatrix5dot0[SPEAKER_COUNT * 5] = 
{ 
	// Split the center channel to the front two speakers
	1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // FC
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // SL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // SR
};

const float OutputMatrix5dot1[SPEAKER_COUNT * 6] = 
{ 
	// Classic 5.1 setup
	1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // FC
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // LFE
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // RL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // RR
};

const float OutputMatrix5dot1s[SPEAKER_COUNT * 6] = 
{ 
	// Classic 5.1 setup
	1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // FC
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // LFE
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // SL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // SR
};

const float OutputMatrix6dot1[SPEAKER_COUNT * 7] = 
{ 
	1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // FC
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // LFE
	0.0f, 0.0f, 0.0f, 0.0f, 0.8f, 0.0f, // RL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.8f, // RR
	0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, // RC
};

const float OutputMatrix7dot1[SPEAKER_COUNT * 8] = 
{ 
	0.7f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 0.7f, 0.0f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 0.7f, 0.0f, 0.0f, 0.0f, // FC
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // LFE
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // RL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // RR
	0.7f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, // FCL
	0.0f, 0.7f, 0.5f, 0.0f, 0.0f, 0.0f  // FCR
};

const float OutputMatrix7dot1s[SPEAKER_COUNT * 8] = 
{ 
	// Split the rear channels evenly between side and rear
	1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FL
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // FR
	0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // FC
	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // LFE
	0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f, // RL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // RR
	0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.0f, // SL
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f  // SR
};

typedef struct SOuputMapping
{
	uint32 NumChannels;
	uint32 SpeakerMask;
	const float* OuputMatrix;
} TOuputMapping;

TOuputMapping OutputMappings[] =
{
	{ 1, SPEAKER_MONO, OutputMatrixMono },
	{ 2, SPEAKER_STEREO, OutputMatrix2dot0 },
	{ 3, SPEAKER_2POINT1, OutputMatrix2dot1 },
	{ 4, SPEAKER_SURROUND, OutputMatrix4dot0s },
	{ 4, SPEAKER_QUAD, OutputMatrix4dot0 },
	{ 5, SPEAKER_4POINT1, OutputMatrix4dot1 },
	{ 5, SPEAKER_5POINT0, OutputMatrix5dot0 },
	{ 6, SPEAKER_5POINT1, OutputMatrix5dot1 },
	{ 6, SPEAKER_5POINT1_SURROUND, OutputMatrix5dot1s },
	{ 7, SPEAKER_6POINT1, OutputMatrix6dot1 },
	{ 8, SPEAKER_7POINT1, OutputMatrix7dot1 },
	{ 8, SPEAKER_7POINT1_SURROUND, OutputMatrix7dot1s }
};

bool FXAudio2Device::GetOutputMatrix( uint32 ChannelMask, uint32 NumChannels )
{
	// Default is vanilla stereo
	FXAudioDeviceProperties::OutputMixMatrix = OutputMatrix2dot0;

	// Find the best match
	for( int32 MappingIndex = 0; MappingIndex < sizeof( OutputMappings ) / sizeof( TOuputMapping ); MappingIndex++ )
	{
		if( NumChannels != OutputMappings[MappingIndex].NumChannels )
		{
			continue;
		}

		if( ( ChannelMask & OutputMappings[MappingIndex].SpeakerMask ) != ChannelMask )
		{
			continue;
		}

		FXAudioDeviceProperties::OutputMixMatrix = OutputMappings[MappingIndex].OuputMatrix;
		break;
	}

	return( FXAudioDeviceProperties::OutputMixMatrix != NULL );
}


/** Test decompress a vorbis file */
void FXAudio2Device::TestDecompressOggVorbis( USoundWave* Wave )
{
	FVorbisAudioInfo	OggInfo;
	FSoundQualityInfo	QualityInfo = { 0 };

	// Parse the ogg vorbis header for the relevant information
	if( OggInfo.ReadCompressedInfo( Wave->ResourceData, Wave->ResourceSize, &QualityInfo ) )
	{
		// Extract the data
		Wave->SampleRate = QualityInfo.SampleRate;
		Wave->NumChannels = QualityInfo.NumChannels;
		Wave->RawPCMDataSize = QualityInfo.SampleDataSize;
		Wave->Duration = QualityInfo.Duration;

		Wave->RawPCMData = ( uint8* )FMemory::Malloc( Wave->RawPCMDataSize );

		// Decompress all the sample data (and preallocate memory)
		OggInfo.ExpandFile( Wave->RawPCMData, &QualityInfo );

		FMemory::Free( Wave->RawPCMData );
	}
}

#if !UE_BUILD_SHIPPING
/** Decompress a wav a number of times for profiling purposes */
void FXAudio2Device::TimeTest( FOutputDevice& Ar, const TCHAR* WaveAssetName )
{
	USoundWave* Wave = LoadObject<USoundWave>( NULL, WaveAssetName, NULL, LOAD_NoWarn, NULL );
	if( Wave )
	{
		// Wait for initial decompress
		if (Wave->AudioDecompressor)
		{
			while (!Wave->AudioDecompressor->IsDone())
			{
			}

			delete Wave->AudioDecompressor;
			Wave->AudioDecompressor = NULL;
		}
		
		// If the wave loaded in fine, time the decompression
		Wave->InitAudioResource(GetRuntimeFormat(Wave));

		double Start = FPlatformTime::Seconds();

		for( int32 i = 0; i < 1000; i++ )
		{
			TestDecompressOggVorbis( Wave );
		} 

		double Duration = FPlatformTime::Seconds() - Start;
		Ar.Logf( TEXT( "%s: %g ms - %g ms per second per channel" ), WaveAssetName, Duration, Duration / ( Wave->Duration * Wave->NumChannels ) );

		Wave->RemoveAudioResource();
	}
	else
	{
		Ar.Logf( TEXT( "Failed to find test file '%s' to decompress" ), WaveAssetName );
	}
}
#endif // !UE_BUILD_SHIPPING

/**
 * Exec handler used to parse console commands.
 *
 * @param	InWorld		World context
 * @param	Cmd		Command to parse
 * @param	Ar		Output device to use in case the handler prints anything
 * @return	true if command was handled, false otherwise
 */
bool FXAudio2Device::Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( FAudioDevice::Exec( InWorld, Cmd, Ar ) )
	{
		return( true );
	}
#if !UE_BUILD_SHIPPING
	else if( FParse::Command( &Cmd, TEXT( "TestVorbisDecompressionSpeed" ) ) )
	{

		TimeTest( Ar, TEXT( "TestSounds.44Mono_TestWeaponSynthetic" ) );
		TimeTest( Ar, TEXT( "TestSounds.44Mono_TestDialogFemale" ) );
		TimeTest( Ar, TEXT( "TestSounds.44Mono_TestDialogMale" ) );

		TimeTest( Ar, TEXT( "TestSounds.22Mono_TestWeaponSynthetic" ) );
		TimeTest( Ar, TEXT( "TestSounds.22Mono_TestDialogFemale" ) );
		TimeTest( Ar, TEXT( "TestSounds.22Mono_TestDialogMale" ) );

		TimeTest( Ar, TEXT( "TestSounds.22Stereo_TestMusicAcoustic" ) );
		TimeTest( Ar, TEXT( "TestSounds.44Stereo_TestMusicAcoustic" ) );
	}
#endif // !UE_BUILD_SHIPPING

	return( false );
}

/**
 * Allocates memory from permanent pool. This memory will NEVER be freed.
 *
 * @param	Size	Size of allocation.
 *
 * @return pointer to a chunk of memory with size Size
 */
void* FXAudio2Device::AllocatePermanentMemory( int32 Size, bool& AllocatedInPool )
{
	void* Allocation = NULL;
	
	// Fall back to using regular allocator if there is not enough space in permanent memory pool.
	if( Size > CommonAudioPoolFreeBytes )
	{
		Allocation = FMemory::Malloc( Size );
		check( Allocation );

		AllocatedInPool = false;
	}
	// Allocate memory from pool.
	else
	{
		uint8* CommonAudioPoolAddress = ( uint8* )CommonAudioPool;
		Allocation = CommonAudioPoolAddress + ( CommonAudioPoolSize - CommonAudioPoolFreeBytes );

		AllocatedInPool = true;
	}

	// Decrement available size regardless of whether we allocated from pool or used regular allocator
	// to allow us to log suggested size at the end of initial loading.
	CommonAudioPoolFreeBytes -= Size;
	
	return( Allocation );
}
