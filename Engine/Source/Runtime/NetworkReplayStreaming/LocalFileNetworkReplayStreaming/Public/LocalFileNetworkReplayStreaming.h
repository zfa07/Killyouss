// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkReplayStreaming.h"
#include "Stats/Stats.h"
#include "Tickable.h"
#include "Serialization/ArrayReader.h"
#include "Serialization/ArrayWriter.h"
#include "Serialization/MemoryReader.h"
#include "Async/Async.h"
#include "Templates/SharedPointer.h"
#include "HAL/ThreadSafeBool.h"

class FNetworkReplayVersion;
class FLocalFileNetworkReplayStreamer;

enum class ELocalFileChunkType : uint32
{
	Header,
	ReplayData,
	Checkpoint,
	Event,
	Unknown = 0xFFFFFFFF
};

/** Struct to hold chunk metadata */
struct FLocalFileChunkInfo
{
	FLocalFileChunkInfo() : 
		ChunkType(ELocalFileChunkType::Unknown),
		SizeInBytes(0),
		TypeOffset(0),
		DataOffset(0)
	{}

	ELocalFileChunkType ChunkType;
	int32 SizeInBytes;
	int64 TypeOffset;
	int64 DataOffset;
};

/** Struct to hold replay data chunk metadata */
struct FLocalFileReplayDataInfo
{
	FLocalFileReplayDataInfo() :
		ChunkIndex(INDEX_NONE),
		Time1(0),
		Time2(0),
		SizeInBytes(0),
		ReplayDataOffset(0),
		StreamOffset(0)
	{}

	int32 ChunkIndex;
	uint32 Time1;
	uint32 Time2;
	int32 SizeInBytes;
	int64 ReplayDataOffset;
	int64 StreamOffset;
};

/** Struct to hold event metadata */
struct FLocalFileEventInfo
{
	FLocalFileEventInfo() : 
		ChunkIndex(INDEX_NONE),
		Time1(0),
		Time2(0),
		SizeInBytes(0),
		EventDataOffset(0)
	{}
	
	int32 ChunkIndex;

	FString Id;
	FString Group;
	FString Metadata;
	uint32 Time1;
	uint32 Time2;

	int32 SizeInBytes;
	int64 EventDataOffset;
};

/** Struct to hold metadata about an entire replay */
struct FLocalFileReplayInfo
{
	FLocalFileReplayInfo() :
		LengthInMS(0), 
		NetworkVersion(0),
		Changelist(0),
		Timestamp(0),
		TotalDataSizeInBytes(0),
		bIsLive(false),
		bIsValid(false),
		bCompressed(false),
		HeaderChunkIndex(INDEX_NONE)
	{}

	int32 LengthInMS;
	uint32 NetworkVersion;
	uint32 Changelist;
	FString	FriendlyName;
	FDateTime Timestamp;
	int64 TotalDataSizeInBytes;
	bool bIsLive;
	bool bIsValid;
	bool bCompressed;

	int32 HeaderChunkIndex;

	TArray<FLocalFileChunkInfo> Chunks;
	TArray<FLocalFileEventInfo> Checkpoints;
	TArray<FLocalFileEventInfo> Events;
	TArray<FLocalFileReplayDataInfo> DataChunks;
};

/** Archive to wrap the file reader and respect chunk boundaries */
class LOCALFILENETWORKREPLAYSTREAMING_API FLocalFileStreamFArchive : public FArchive
{
public:
	FLocalFileStreamFArchive() : Pos( 0 ), bAtEndOfReplay( false ) {}

	virtual void	Serialize( void* V, int64 Length ) override;
	virtual int64	Tell() override;
	virtual int64	TotalSize() override;
	virtual void	Seek( int64 InPos ) override;
	virtual bool	AtEnd() override;

	TArray< uint8 >	Buffer;
	int32			Pos;
	bool			bAtEndOfReplay;
};

namespace EQueuedLocalFileRequestType
{
	enum Type
	{
		StartRecording,
		WriteHeader,
		WritingHeader,
		WritingStream,
		StopRecording,
		StartPlayback,
		ReadingHeader,
		ReadingStream,
		EnumeratingStreams,
		WritingCheckpoint,
		ReadingCheckpoint,
		UpdatingEvent,
		EnumeratingEvents,
		RequestingEvent,
		StopStreaming,	
		DeletingFinishedStream,
		RefreshingLiveStream,
		KeepReplay,
		RenameReplay,
		RenameReplayFriendlyName,
	};

	inline const TCHAR* ToString( EQueuedLocalFileRequestType::Type Type )
	{
		switch ( Type )
		{
		case StartRecording:
			return TEXT( "StartRecording" );
		case WriteHeader:
			return TEXT( "WriteHeader" );
		case WritingHeader:
			return TEXT( "WritingHeader" );
		case WritingStream:
			return TEXT( "WritingStream" );
		case StopRecording:
			return TEXT( "StopRecording" );
		case StartPlayback:
			return TEXT( "StartPlayback" );
		case ReadingHeader:
			return TEXT( "ReadingHeader" );
		case ReadingStream:
			return TEXT( "ReadingStream" );
		case EnumeratingStreams:
			return TEXT( "EnumeratingStreams" );
		case WritingCheckpoint:
			return TEXT( "WritingCheckpoint" );
		case ReadingCheckpoint:
			return TEXT( "ReadingCheckpoint" );
		case UpdatingEvent:
			return TEXT( "UpdatingEvent" );
		case EnumeratingEvents:
			return TEXT( "EnumeratingEvents" );
		case RequestingEvent:
			return TEXT("RequestingEvent");
		case StopStreaming:
			return TEXT( "StopStreaming" );
		case DeletingFinishedStream:
			return TEXT( "DeletingFinishedStream" );
		case RefreshingLiveStream:
			return TEXT( "RefreshingLiveStream" );
		case KeepReplay:
			return TEXT( "KeepReplay" );
		case RenameReplay:
			return TEXT( "RenameReplay" );
		case RenameReplayFriendlyName:
			return TEXT( "RenameReplayFriendlyName" );
		}

		return TEXT( "Unknown EQueuedLocalFileRequestType type." );
	}
};

class FCachedFileRequest
{
public:
	FCachedFileRequest( const TArray<uint8>& InRequestData, const double InLastAccessTime ) : RequestData( InRequestData ), LastAccessTime( InLastAccessTime )
	{
	}

	TArray<uint8> RequestData;
	double LastAccessTime;
};

class FQueuedLocalFileRequest
{
public:
	FQueuedLocalFileRequest(const TSharedPtr<FLocalFileNetworkReplayStreamer>& InStreamer, EQueuedLocalFileRequestType::Type InType)
		: Streamer(InStreamer)
		, RequestType(InType)
		, bCancelled(false)
	{
	}

	virtual ~FQueuedLocalFileRequest() {}

	EQueuedLocalFileRequestType::Type GetRequestType() const { return RequestType; }

	virtual bool GetCachedRequest() { return false; }
	virtual void IssueRequest() = 0;
	virtual void FinishRequest() = 0;

	void CancelRequest();

protected:
	TSharedPtr<FLocalFileNetworkReplayStreamer> Streamer;
	EQueuedLocalFileRequestType::Type RequestType;
	FThreadSafeBool bCancelled;	
};

class FGenericQueuedLocalFileRequest : public FQueuedLocalFileRequest, public TSharedFromThis<FGenericQueuedLocalFileRequest, ESPMode::ThreadSafe>
{
public:
	FGenericQueuedLocalFileRequest(const TSharedPtr<FLocalFileNetworkReplayStreamer>& InStreamer, EQueuedLocalFileRequestType::Type InType, TFunction<void()>& InFunction, TFunction<void()>& InCompletionCallback)
		: FQueuedLocalFileRequest(InStreamer, InType)
		, RequestFunction(MoveTemp(InFunction))
		, CompletionCallback(MoveTemp(InCompletionCallback))
	{
	}

	virtual void IssueRequest() override;
	virtual void FinishRequest() override;

protected:
	TFunction<void()> RequestFunction;
	TFunction<void()> CompletionCallback;
};

template <typename StorageType>
class TGenericQueuedLocalFileRequest : public FQueuedLocalFileRequest, public TSharedFromThis<TGenericQueuedLocalFileRequest<StorageType>, ESPMode::ThreadSafe>
{
public:
	TGenericQueuedLocalFileRequest(const TSharedPtr<FLocalFileNetworkReplayStreamer>& InStreamer, EQueuedLocalFileRequestType::Type InType, TFunction<void(StorageType&)>& InFunction, TFunction<void(StorageType&)>& InCompletionCallback)
		: FQueuedLocalFileRequest(InStreamer, InType)
		, RequestFunction(MoveTemp(InFunction))
		, CompletionCallback(MoveTemp(InCompletionCallback))
	{
	}

	virtual void IssueRequest() override
	{
		auto SharedRef = this->AsShared();

		Async<void>(EAsyncExecution::TaskGraph, 
			[SharedRef]()
			{
				SharedRef->RequestFunction(SharedRef->Storage);
			},
			[SharedRef]() 
			{
				if (!SharedRef->bCancelled)
				{
					AsyncTask(ENamedThreads::GameThread, [SharedRef]()
					{
						SharedRef->FinishRequest();
					});
				}
			});
	}

	virtual void FinishRequest() override
	{
		if (CompletionCallback)
		{
			CompletionCallback(Storage);
		}

		if (!bCancelled && this->Streamer.IsValid())
		{
			this->Streamer->OnFileRequestComplete(this->AsShared());
		}
	}

	StorageType Storage;

protected:
	TFunction<void(StorageType&)> RequestFunction;
	TFunction<void(StorageType&)> CompletionCallback;
};

template <typename DelegateResultType>
class TLocalFileRequestCommonData
{
public:
	DelegateResultType DelegateResult;
	FLocalFileReplayInfo ReplayInfo;
	TArray<uint8> DataBuffer;
};

template <typename DelegateResultType>
class TGenericCachedLocalFileRequest : public TGenericQueuedLocalFileRequest<TLocalFileRequestCommonData<DelegateResultType>>
{
public:
	TGenericCachedLocalFileRequest(int32 InCacheKey, const TSharedPtr<FLocalFileNetworkReplayStreamer>& InStreamer, EQueuedLocalFileRequestType::Type InType, TFunction<void(TLocalFileRequestCommonData<DelegateResultType>&)>& InFunction, TFunction<void(TLocalFileRequestCommonData<DelegateResultType>&)>& InCompletionCallback)
		: TGenericQueuedLocalFileRequest<TLocalFileRequestCommonData<DelegateResultType>>(InStreamer, InType, InFunction, InCompletionCallback)
		, CacheKey(InCacheKey)
	{}

	virtual bool GetCachedRequest() override
	{
		FCachedFileRequest* CachedRequest = this->Streamer->RequestCache.Find(CacheKey);
		if (CachedRequest != nullptr)
		{
			// If we have this response in the cache, process it now
			CachedRequest->LastAccessTime = FPlatformTime::Seconds();
			this->Storage.DataBuffer = CachedRequest->RequestData;
			return true;
		}

		return false; 
	}

protected:
	int32 CacheKey;
};

/** Local file streamer that supports playback/recording to a single file on disk */
class LOCALFILENETWORKREPLAYSTREAMING_API FLocalFileNetworkReplayStreamer : public INetworkReplayStreamer, public TSharedFromThis<FLocalFileNetworkReplayStreamer>
{
public:
	FLocalFileNetworkReplayStreamer();
	FLocalFileNetworkReplayStreamer(const FString& InDemoSavePath);
	virtual ~FLocalFileNetworkReplayStreamer();

	/** INetworkReplayStreamer implementation */
	virtual void StartStreaming(const FString& CustomName, const FString& FriendlyName, const TArray< FString >& UserNames, bool bRecord, const FNetworkReplayVersion& ReplayVersion, const FStartStreamingCallback& Delegate) override;
	virtual void StartStreaming(const FString& CustomName, const FString& FriendlyName, const TArray< int32 >& UserIndices, bool bRecord, const FNetworkReplayVersion& ReplayVersion, const FStartStreamingCallback& Delegate) override;
	virtual void StopStreaming() override;
	virtual FArchive* GetHeaderArchive() override;
	virtual FArchive* GetStreamingArchive() override;
	virtual FArchive* GetCheckpointArchive() override;
	virtual void FlushCheckpoint(const uint32 TimeInMS) override;
	virtual void GotoCheckpointIndex(const int32 CheckpointIndex, const FGotoCallback& Delegate) override;
	virtual void GotoTimeInMS(const uint32 TimeInMS, const FGotoCallback& Delegate) override;
	virtual void UpdateTotalDemoTime(uint32 TimeInMS) override;
	virtual uint32 GetTotalDemoTime() const override { return CurrentReplayInfo.LengthInMS; }
	virtual bool IsDataAvailable() const override;
	virtual void SetHighPriorityTimeRange(const uint32 StartTimeInMS, const uint32 EndTimeInMS) override;
	virtual bool IsDataAvailableForTimeRange(const uint32 StartTimeInMS, const uint32 EndTimeInMS) override;
	virtual bool IsLoadingCheckpoint() const override;
	virtual bool IsLive() const override;
	virtual void DeleteFinishedStream(const FString& StreamName, const FDeleteFinishedStreamCallback& Delegate) override;
	virtual void DeleteFinishedStream( const FString& StreamName, const int32 UserIndex, const FDeleteFinishedStreamCallback& Delegate ) override;
	virtual void EnumerateStreams(const FNetworkReplayVersion& ReplayVersion, const FString& UserString, const FString& MetaString, const FEnumerateStreamsCallback& Delegate) override;
	virtual void EnumerateStreams(const FNetworkReplayVersion& InReplayVersion, const FString& UserString, const FString& MetaString, const TArray< FString >& ExtraParms, const FEnumerateStreamsCallback& Delegate) override;
	virtual void EnumerateStreams( const FNetworkReplayVersion& InReplayVersion, const int32 UserIndex, const FString& MetaString, const TArray< FString >& ExtraParms, const FEnumerateStreamsCallback& Delegate ) override;
	virtual void EnumerateRecentStreams(const FNetworkReplayVersion& ReplayVersion, const FString& RecentViewer, const FEnumerateStreamsCallback& Delegate) override;
	virtual void EnumerateRecentStreams( const FNetworkReplayVersion& ReplayVersion, const int32 UserIndex, const FEnumerateStreamsCallback& Delegate ) override;
	virtual ENetworkReplayError::Type GetLastError() const override;
	virtual void AddUserToReplay(const FString& UserString) override;
	virtual void AddEvent(const uint32 TimeInMS, const FString& Group, const FString& Meta, const TArray<uint8>& Data) override;
	virtual void AddOrUpdateEvent(const FString& Name, const uint32 TimeInMS, const FString& Group, const FString& Meta, const TArray<uint8>& Data) override;
	virtual void EnumerateEvents(const FString& Group, const FEnumerateEventsCallback& Delegate) override;
	virtual void EnumerateEvents(const FString& ReplayName, const FString& Group, const FEnumerateEventsCallback& Delegate) override;
	virtual void EnumerateEvents( const FString& ReplayName, const FString& Group, const int32 UserIndex, const FEnumerateEventsCallback& Delegate ) override;
	virtual void RequestEventData(const FString& EventID, const FRequestEventDataCallback& Delegate) override;
	virtual void RequestEventData(const FString& ReplayName, const FString& EventID, const FRequestEventDataCallback& Delegate) override;
	virtual void RequestEventData(const FString& ReplayName, const FString& EventId, const int32 UserIndex, const FRequestEventDataCallback& Delegate) override;
	virtual void SearchEvents(const FString& EventGroup, const FSearchEventsCallback& Delegate) override;
	virtual void KeepReplay(const FString& ReplayName, const bool bKeep, const FKeepReplayCallback& Delegate) override;
	virtual void KeepReplay(const FString& ReplayName, const bool bKeep, const int32 UserIndex, const FKeepReplayCallback& Delegate) override;
	virtual void RenameReplayFriendlyName(const FString& ReplayName, const FString& NewFriendlyName, const FRenameReplayCallback& Delegate) override;
	virtual void RenameReplayFriendlyName(const FString& ReplayName, const FString& NewFriendlyName, const int32 UserIndex, const FRenameReplayCallback& Delegate) override;
	virtual void RenameReplay(const FString& ReplayName, const FString& NewName, const FRenameReplayCallback& Delegate) override;
	virtual void RenameReplay(const FString& ReplayName, const FString& NewName, const int32 UserIndex, const FRenameReplayCallback& Delegate) override;
	virtual FString	GetReplayID() const override { return TEXT(""); }
	virtual void SetTimeBufferHintSeconds(const float InTimeBufferHintSeconds) override {}
	virtual void RefreshHeader() override;
	virtual void DownloadHeader(const FDownloadHeaderCallback& Delegate) override;

	virtual bool SupportsCompression() const { return false; }
	virtual int32 GetDecompressedSize(FArchive& InCompressed) const { return 0; }
	virtual bool DecompressBuffer(const TArray<uint8>& InCompressed, TArray< uint8 >& OutBuffer) const { return false; }
	virtual bool CompressBuffer(const TArray< uint8 >& InBuffer, TArray< uint8 >& OutCompressed) const { return false; }

	void Tick(float DeltaSeconds);

	virtual uint32 GetMaxFriendlyNameSize() const override;

	virtual EStreamingOperationResult SetDemoPath(const FString& DemoPath) override
	{
		if (CurrentStreamName.IsEmpty())
		{
			DemoSavePath = DemoPath;
			return EStreamingOperationResult::Success;
		}
		else
		{
			return EStreamingOperationResult::Unspecified;
		}
	}

	virtual EStreamingOperationResult GetDemoPath(FString& DemoPath) const override
	{
		DemoPath = DemoSavePath;
		return EStreamingOperationResult::Success;
	}

	void OnFileRequestComplete(const TSharedPtr<FQueuedLocalFileRequest, ESPMode::ThreadSafe>& Request);

	bool IsStreaming() const;

	bool HasPendingFileRequests() const;

	void AddSimpleRequestToQueue(EQueuedLocalFileRequestType::Type RequestType, TFunction<void()>&& InFunction, TFunction<void()>&& InCompletionCallback)
	{
		QueuedRequests.Add(MakeShared<FGenericQueuedLocalFileRequest, ESPMode::ThreadSafe>(AsShared(), RequestType, InFunction, InCompletionCallback));
	}

	template <typename StorageType>
	void AddGenericRequestToQueue(EQueuedLocalFileRequestType::Type RequestType, TFunction<void(StorageType&)>&& InFunction, TFunction<void(StorageType&)>&& InCompletionCallback)
	{
		QueuedRequests.Add(MakeShared<TGenericQueuedLocalFileRequest<StorageType>, ESPMode::ThreadSafe>(AsShared(), RequestType, InFunction, InCompletionCallback));
	}

	template<typename DelegateResultType>
	void AddDelegateFileRequestToQueue(EQueuedLocalFileRequestType::Type RequestType, TFunction<void(TLocalFileRequestCommonData<DelegateResultType>&)>&& InFunction, TFunction<void(TLocalFileRequestCommonData<DelegateResultType>&)>&& InCompletionCallback)
	{
		AddGenericRequestToQueue<TLocalFileRequestCommonData<DelegateResultType>>(RequestType, MoveTemp(InFunction), MoveTemp(InCompletionCallback));
	}

	template<typename DelegateType, typename DelegateResultType>
	void AddDelegateFileRequestToQueue(EQueuedLocalFileRequestType::Type RequestType, const DelegateType& Delegate, TFunction<void(TLocalFileRequestCommonData<DelegateResultType>&)>&& InFunction)
	{
		AddGenericRequestToQueue<TLocalFileRequestCommonData<DelegateResultType>>(RequestType, MoveTemp(InFunction), 
			[Delegate](TLocalFileRequestCommonData<DelegateResultType>& Storage) 
			{ 
				Delegate.ExecuteIfBound(Storage.DelegateResult); 
			});
	}

	template<typename DelegateResultType>
	void AddCachedFileRequestToQueue(EQueuedLocalFileRequestType::Type RequestType, int32 InCacheKey, TFunction<void(TLocalFileRequestCommonData<DelegateResultType>&)>&& InFunction, TFunction<void(TLocalFileRequestCommonData<DelegateResultType>&)>&& InCompletionCallback)
	{
		QueuedRequests.Add(MakeShared<TGenericCachedLocalFileRequest<DelegateResultType>, ESPMode::ThreadSafe>(InCacheKey, AsShared(), RequestType, InFunction, InCompletionCallback));
	}

	/** Map of chunk index to cached value */
	TMap<int32, FCachedFileRequest> RequestCache;

protected:

	static const FString GetUserStringFromUserIndex(const int32 UserIndex);
	static const void GetUserStringsFromUserIndices(const TArray<int32>& UserIndices, TArray<FString>& OutUserStrings);

	static const int32 GetUserIndexFromUserString(const FString& UserString);
	static const void GetUserIndicesFromUserStrings(const TArray<FString>& UserStrings, TArray<int32>& OutUserIndices);

	void StartStreaming_Internal(const FString& CustomName, const FString& FriendlyName, const TArray<int32>& UserIndices, bool bRecord, const FNetworkReplayVersion& ReplayVersion, const FStartStreamingCallback& Delegate);
	void DeleteFinishedStream_Internal(const FString& StreamName, const int32 UserIndex, const FDeleteFinishedStreamCallback& Delegate);
	void EnumerateStreams_Internal(const FNetworkReplayVersion& InReplayVersion, const int32 UserIndex, const FString& MetaString, const TArray< FString >& ExtraParms, const FEnumerateStreamsCallback& Delegate);
	void EnumerateRecentStreams_Internal(const FNetworkReplayVersion& ReplayVersion, const int32 UserIndex, const FEnumerateStreamsCallback& Delegate);
	void EnumerateEvents_Internal(const FString& ReplayName, const FString& Group, const int32 UserIndex, const FEnumerateEventsCallback& Delegate);
	void RequestEventData_Internal(const FString& ReplayName, const FString& EventId, const int32 UserIndex, const FRequestEventDataCallback& Delegate);
	void KeepReplay_Internal(const FString& ReplayName, const bool bKeep, const int32 UserIndex, const FKeepReplayCallback& Delegate);
	void RenameReplayFriendlyName_Internal(const FString& ReplayName, const FString& NewFriendlyName, const int32 UserIndex, const FRenameReplayCallback& Delegate);
	void RenameReplay_Internal(const FString& ReplayName, const FString& NewName, const int32 UserIndex, const FRenameReplayCallback& Delegate);

	/** Currently playing or recording replay metadata */
	FLocalFileReplayInfo CurrentReplayInfo;

	TInterval<uint32> StreamTimeRange;
	int64 StreamDataOffset;

	int32 StreamChunkIndex;
	double LastChunkTime;
	double LastRefreshTime;
	bool bStopStreamingCalled;
	uint32 HighPriorityEndTime;
	int64 LastGotoTimeInMS;

	TArray<TSharedPtr<FQueuedLocalFileRequest, ESPMode::ThreadSafe>> QueuedRequests;
	TSharedPtr<FQueuedLocalFileRequest, ESPMode::ThreadSafe> ActiveRequest;

	bool ProcessNextFileRequest();
	bool IsFileRequestInProgress() const;
	bool IsFileRequestPendingOrInProgress(const EQueuedLocalFileRequestType::Type RequestType) const;
	void CancelStreamingRequests();

	void SetLastError(const ENetworkReplayError::Type InLastError);

	void ConditionallyFlushStream();
	void ConditionallyLoadNextChunk();
	void ConditionallyRefreshReplayInfo();

	void FlushCheckpointInternal(const uint32 TimeInMS);

	struct FLocalFileSerializationInfo
	{
		FLocalFileSerializationInfo();

		uint32 FileVersion;
	};

	bool ReadReplayInfo(const FString& StreamName, FLocalFileReplayInfo& OutReplayInfo) const;
	bool ReadReplayInfo(FArchive& Archive, FLocalFileReplayInfo& OutReplayInfo) const;
	bool ReadReplayInfo(FArchive& Archive, FLocalFileReplayInfo& OutReplayInfo, struct FLocalFileSerializationInfo& SerializationInfo) const;

	bool WriteReplayInfo(const FString& StreamName, const FLocalFileReplayInfo& ReplayInfo);
	bool WriteReplayInfo(FArchive& Archive, const FLocalFileReplayInfo& ReplayInfo);
	bool WriteReplayInfo(FArchive& Archive, const FLocalFileReplayInfo& InReplayInfo, struct FLocalFileSerializationInfo& SerializationInfo);

	void FixupFriendlyNameLength(const FString& UnfixedName, FString& FixedName) const;

	bool IsNamedStreamLive(const FString& StreamName) const;

	void FlushStream(const uint32 TimeInMS);

	void WriteHeader();

	virtual TSharedPtr<FArchive> CreateLocalFileReader(const FString& InFilename) const;
	virtual TSharedPtr<FArchive> CreateLocalFileWriter(const FString& InFilename) const;
	virtual TSharedPtr<FArchive> CreateLocalFileWriterForOverwrite(const FString& InFilename) const;

	FString GetDemoPath() const;
	FString GetDemoFullFilename(const FString& FileName) const;

	// Returns a name formatted as "demoX", where X is between 1 and MAX_DEMOS, inclusive.
	// Returns the first value that doesn't yet exist, or if they all exist, returns the oldest one
	// (it will be overwritten).
	FString GetAutomaticDemoName() const;

	/** Handle to the archive that will read/write the demo header */
	FLocalFileStreamFArchive HeaderAr;

	/** Handle to the archive that will read/write network packets */
	FLocalFileStreamFArchive StreamAr;

	/* Handle to the archive that will read/write checkpoint files */
	FLocalFileStreamFArchive CheckpointAr;

	/** EStreamerState - Overall state of the streamer */
	enum class EStreamerState
	{
		Idle,					// The streamer is idle. Either we haven't started streaming yet, or we are done
		Recording,				// We are in the process of recording a replay to disk
		Playback,				// We are in the process of playing a replay from disk
	};

	/** Overall state of the streamer */
	EStreamerState StreamerState;

	ENetworkReplayError::Type StreamerLastError;

	/** Remember the name of the current stream, if any. */
	FString CurrentStreamName;

	FString DemoSavePath;

	void AddRequestToCache(int32 ChunkIndex, const TArray<uint8>& RequestData);
	void CleanupRequestCache();

public:
	static const FString& GetDefaultDemoSavePath();
};

class LOCALFILENETWORKREPLAYSTREAMING_API FLocalFileNetworkReplayStreamingFactory : public INetworkReplayStreamingFactory, public FTickableGameObject
{
public:
	virtual TSharedPtr<INetworkReplayStreamer> CreateReplayStreamer();

	/** FTickableGameObject */
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
	bool IsTickableWhenPaused() const override { return true; }

protected:
	TArray<TSharedPtr<FLocalFileNetworkReplayStreamer>> LocalFileStreamers;
};
