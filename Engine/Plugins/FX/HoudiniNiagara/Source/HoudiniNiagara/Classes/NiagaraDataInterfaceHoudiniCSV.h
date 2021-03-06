/*
* Copyright (c) <2017> Side Effects Software Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "NiagaraCommon.h"
#include "NiagaraShared.h"
#include "VectorVM.h"
#include "HoudiniCSV.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceHoudiniCSV.generated.h"


/** Data Interface allowing sampling of Houdini CSV files. */
UCLASS(EditInlineNew, Category = "Houdini Niagara", meta = (DisplayName = "Houdini CSV File"))
class HOUDININIAGARA_API UNiagaraDataInterfaceHoudiniCSV : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()
public:

	//UPROPERTY( EditAnywhere, Category = "Houdini Niagara", meta = (DisplayName = "Houdini CSV File Path" ) )
	//FFilePath CSVFileName;

	UPROPERTY( EditAnywhere, Category = "Houdini Niagara", meta = (DisplayName = "Houdini CSV File" ) )
	UHoudiniCSV* CSVFile;

	//void UpdateDataFromCSVFile();

	//----------------------------------------------------------------------------
	//UObject Interface
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//----------------------------------------------------------------------------

	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)override;
	
	/** Returns the delegate for the passed function signature. */
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc) override;

	virtual bool Equals(const UNiagaraDataInterface* Other) const override;

	//----------------------------------------------------------------------------
	// EXPOSED FUNCTIONS

	// Returns the float value at a given point in the CSV file
	template<typename RowParamType, typename ColParamType>
	void GetCSVFloatValue(FVectorVMContext& Context);

	template<typename RowParamType, typename ColTitleParamType>
	void GetCSVFloatValueByString(FVectorVMContext& Context);

	// Returns a Vector3 value for a given point in the csv file
	template<typename RowParamType, typename ColParamType>
	void GetCSVVectorValue(FVectorVMContext& Context);

	// Returns the positions for a given point in the CSV file
	template<typename NParamType>
	void GetCSVPosition(FVectorVMContext& Context);

	// Returns the normals for a given point in the CSV file
	template<typename NParamType>
	void GetCSVNormal(FVectorVMContext& Context);

	// Returns the time for a given point in the CSV file
	template<typename NParamType>
	void GetCSVTime(FVectorVMContext& Context);

	// Returns the position and time for a given point in the CSV file
	template<typename NParamType>
	void GetCSVPositionAndTime(FVectorVMContext& Context);

	// Returns the number of points found in the CSV file
	void GetNumberOfPointsInCSV(FVectorVMContext& Context);

	// Returns the last index of the particles that should be spawned at time t
	template<typename TimeParamType>
	void GetLastParticleIndexAtTime(FVectorVMContext& Context);

	// Returns the indexes (min, max) and number of particles that should be spawned at time t
	template<typename TimeParamType>
	void GetParticleIndexesToSpawnAtTime(FVectorVMContext& Context);
	
	//----------------------------------------------------------------------------
	// GPU / HLSL Functions
	virtual bool GetFunctionHLSL(const FName& DefinitionFunctionName, FString InstanceFunctionName, FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
	virtual void GetParameterDefinitionHLSL(FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;

	// Disabling GPU sim for now.
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target)const override { return Target == ENiagaraSimTarget::CPUSim; }

protected:

	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;

	// Indicates the GPU buffers need to be updated
	UPROPERTY()
	bool GPUBufferDirty;

	// Last Index used to spawn particles
	UPROPERTY()
	int32 LastSpawnIndex;
};
