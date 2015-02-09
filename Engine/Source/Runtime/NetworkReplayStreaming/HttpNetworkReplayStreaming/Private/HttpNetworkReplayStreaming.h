// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "NetworkReplayStreaming.h"
#include "HTTP.h"
#include "Runtime/Engine/Public/Tickable.h"

/**
 * Archive used to buffer stream over http
 */
class HttpStreamFArchive : public FArchive
{
public:
	HttpStreamFArchive() : Pos( 0 ) {}

	virtual void	Serialize( void* V, int64 Length );
	virtual int64	Tell();
	virtual int64	TotalSize();
	virtual void	Seek( int64 InPos );

	TArray< uint8 >	Buffer;
	int32			Pos;
};

/**
 * Http network replay streaming manager
 */
class FHttpNetworkReplayStreamer : public INetworkReplayStreamer
{
public:
	/** INetworkReplayStreamer implementation */
	FHttpNetworkReplayStreamer() : StreamFileCount( 0 ), LastFlushTime( 0 ), StreamState( EStreamState::Idle ), bFinalFlushNeeded( false ), NumDownloadChunks( 0 ) {}
	virtual void StartStreaming( FString& StreamName, bool bRecord, const FOnStreamReadyDelegate& Delegate ) override;
	virtual void StopStreaming() override;
	virtual FArchive* GetStreamingArchive() override;
	virtual FArchive* GetMetadataArchive() override;

	/** FHttpNetworkReplayStreamer */
	void FlushStream();
	void DownloadNextChunk();

	void HttpStartDownloadingFinished( FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded );
	void HttpDownloadFinished( FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded );
	void HttpStartUploadingFinished( FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded );
	void HttpStopUploadingFinished( FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded );
	void HttpUploadFinished( FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded );

	void Tick( float DeltaTime );

	bool IsBusy();

	enum class EStreamState
	{
		Idle,
		StartUploading,
		UploadingStream,
		StopUploading,
		StartDownloading,
		DownloadingStream,
	};

	FOnStreamReadyDelegate	RememberedDelegate;		// Delegate passed in to StartStreaming
	HttpStreamFArchive		StreamArchive;			// Archive used to buffer the stream
	FString					SessionName;			// Name of the session on the http replay server
	FString					ServerURL;
	int32					StreamFileCount;		// Used as a counter to increment the stream.x extension count
	double					LastFlushTime;
	EStreamState			StreamState;
	bool					bFinalFlushNeeded;
	int32					NumDownloadChunks;
};

class FHttpNetworkReplayStreamingFactory : public INetworkReplayStreamingFactory, public FTickableGameObject
{
public:
	/** INetworkReplayStreamingFactory */
	virtual TSharedPtr< INetworkReplayStreamer > CreateReplayStreamer();

	/** FTickableGameObject */
	virtual void Tick( float DeltaTime ) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	TArray< TSharedPtr< FHttpNetworkReplayStreamer > > HttpStreamers;
};
