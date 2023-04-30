// Copyright Epic Games, Inc. All Rights Reserved.

#include "WalletAdapter.h"
#include "Android/LocalAssociationIntentCreator.h"
#include "MobileWalletAdapterClientBP.h"

void FWalletAdapterModule::StartupModule()
{
#if PLATFORM_ANDROID
	FLocalAssociationIntentCreator::StaticConstruct();
#endif
}

void FWalletAdapterModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWalletAdapterModule, WalletAdapter)