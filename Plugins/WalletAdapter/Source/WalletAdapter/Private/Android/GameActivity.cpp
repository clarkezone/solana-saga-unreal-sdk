﻿//
// Copyright (c) 2023 Solana Mobile Inc.
// Author: Sergey Makovkin (makovkin.s@gmail.com)
//

#include "GameActivity.h"

#if PLATFORM_ANDROID
#include "Android/JavaClassObjectWrapper.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidPlatform.h"
#include "Android/AndroidJavaEnv.h"
#include "Android/AndroidJNI.h"

BEGIN_IMPLEMENT_JAVA_CLASS_OBJECT(FGameActivity, FJavaClassObjectWrapper, "com/epicgames/unreal/GameActivity", "()V")
	StartActivityMethod = GetClassMethod("startActivity", "(Landroid/content/Intent;)V");
	StartActivityForResultMethod = GetClassMethod("startActivityForResult", "(Landroid/content/Intent;I)V");
END_IMPLEMENT_JAVA_CLASS_OBJECT

void FGameActivity::StartActivity(const FJavaClassObjectWrapperRef& Intent, TSharedPtr<FThrowable>* OutException)
{
	if (OutException)
	{
		jthrowable JThrowable;
		CallThrowableMethod<void>(JThrowable, StartActivityForResultMethod, **Intent);
		*OutException = JThrowable ? MakeShareable(FThrowable::Construct(JThrowable)) : nullptr;
	}
	else
	{
		CallMethod<void>(StartActivityMethod, **Intent);
	}
}

void FGameActivity::StartActivityForResult(const FJavaClassObjectWrapperRef& Intent, int32 RequestCode, TSharedPtr<FThrowable>* OutException)
{
	if (OutException)
	{
		jthrowable JThrowable;
		CallThrowableMethod<void>(JThrowable, StartActivityForResultMethod, **Intent, RequestCode);
		*OutException = JThrowable ? MakeShareable(FThrowable::Construct(JThrowable)) : nullptr;		
	}
	else
	{
		CallMethod<void>(StartActivityForResultMethod, **Intent, RequestCode);
	}
}

#endif
