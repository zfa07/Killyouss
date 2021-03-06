// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PyWrapperBase.h"
#include "UObject/Class.h"
#include "PyWrapperEnum.generated.h"

#if WITH_PYTHON

/** Python type for FPyWrapperEnum */
extern PyTypeObject PyWrapperEnumType;

/** Initialize the PyWrapperEnum types and add them to the given Python module */
void InitializePyWrapperEnum(PyGenUtil::FNativePythonModule& ModuleInfo);

/** Type for all UE4 exposed enum instances */
struct FPyWrapperEnum : public FPyWrapperBase
{
	/** Initialize this wrapper instance (called via tp_init for Python, or directly in C++) */
	static int Init(FPyWrapperEnum* InSelf);

	/** Set the given enum entry value on the given enum type */
	static void SetEnumEntryValue(PyTypeObject* InType, const int64 InEnumValue, const char* InEnumValueName, const char* InEnumValueDoc);
};

/** Meta-data for all UE4 exposed enum types */
struct FPyWrapperEnumMetaData : public FPyWrapperBaseMetaData
{
	PY_METADATA_METHODS(FPyWrapperEnumMetaData, FGuid(0x1D69987C, 0x2F624403, 0x8379FCB5, 0xF896B595))

	FPyWrapperEnumMetaData();

	/** Get the UEnum from the given type */
	static UEnum* GetEnum(PyTypeObject* PyType);

	/** Check to see if the enum is deprecated, and optionally return its deprecation message */
	static bool IsEnumDeprecated(PyTypeObject* PyType, FString* OutDeprecationMessage = nullptr);

	/** Unreal enum */
	UEnum* Enum;

	/** Set if this struct is deprecated and using it should emit a deprecation warning */
	TOptional<FString> DeprecationMessage;
};

typedef TPyPtr<FPyWrapperEnum> FPyWrapperEnumPtr;

#endif	// WITH_PYTHON

/** An Unreal enum that was generated from a Python type */
UCLASS()
class UPythonGeneratedEnum : public UEnum
{
	GENERATED_BODY()

#if WITH_PYTHON

public:
	/** Generate an Unreal enum from the given Python type */
	static UPythonGeneratedEnum* GenerateEnum(PyTypeObject* InPyType);

private:
	/** Definition data for an Unreal enum value generated from a Python type */
	struct FEnumValueDef
	{
		/** Numeric value of the enum value */
		int64 Value;

		/** Name of the enum value */
		FString Name;

		/** Documentation string of the enum value */
		FString DocString;
	};

	/** Python type this enum was generated from */
	FPyTypeObjectPtr PyType;

	/** Array of values generated for this enum */
	TArray<TSharedPtr<FEnumValueDef>> EnumValueDefs;

	/** Meta-data for this generated enum that is applied to the Python type */
	FPyWrapperEnumMetaData PyMetaData;

	friend struct FPythonGeneratedEnumUtil;

#endif	// WITH_PYTHON
};
