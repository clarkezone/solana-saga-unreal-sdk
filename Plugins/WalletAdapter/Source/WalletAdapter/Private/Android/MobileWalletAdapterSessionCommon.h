﻿//
// Copyright (c) 2023 Solana Mobile Inc.
// Author: Sergey Makovkin (makovkin.s@gmail.com)
//

#pragma once

#include "Defines.h"
#include "CoreMinimal.h"
#if PLATFORM_ANDROID
#include "Android/JavaClassObjectWrapper.h"

/**
 * Wrapper for com.solana.mobilewalletadapter.common.protocol.MobileWalletAdapterSessionCommon 
 */
class FMobileWalletAdapterSessionCommon : public FJavaClassObjectWrapper
{
	DECLARE_JAVA_CLASS_OBJECT(FMobileWalletAdapterSessionCommon);
};

#endif
