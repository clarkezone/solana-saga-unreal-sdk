﻿#pragma once

#include "CoreMinimal.h"
#if PLATFORM_ANDROID
#include "Android/JavaClassObjectWrapper.h"
#include "Defines.h"

class FMobileWalletAdapterSession;

/**
 * Wrapper for com.solana.mobilewalletadapter.clientlib.scenario.FLocalAssociationIntentCreator 
 */
class FLocalAssociationIntentCreator	
{
private:
	FLocalAssociationIntentCreator() {}
public:
	/** Initialize java objects and cache them for further usage. Called when the module is loaded. */
	static void StaticConstruct();	
	
	static FGlobalJavaClassObjectRef CreateAssociationIntent(const FString& EndpointPrefix, int32 Port, const FMobileWalletAdapterSession& Session);	
};


#endif