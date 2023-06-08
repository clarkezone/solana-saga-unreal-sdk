﻿//
// Copyright (c) 2023 Solana Mobile Inc.
// Author: Sergey Makovkin (makovkin.s@gmail.com)
//

#include "WalletAdapterClient.h"
#include "WalletAdapter.h"
#include "Android/GameActivity.h"
#include "Android/LocalAssociationScenario.h"
#include "Android/MobileWalletAdapterClient.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#endif

#if PLATFORM_ANDROID
void UWalletAdapterClient::SetClientImpl(const TSharedPtr<FMobileWalletAdapterClient>& InClient)
{
	Client = InClient;
}
#endif

void UWalletAdapterClient::Authorize(FString IdentityUri, FString IconUri, FString IdentityName, FString Cluster, const FAuthSuccessDelegate& Success, const FFailureDelegate& Failure)
{
#if PLATFORM_ANDROID
    TSharedPtr<FThrowable> Exception;

	auto JAuthFuture = Client->Authorize(IdentityUri, IconUri, IdentityName, Cluster, Exception);
	if (Exception)
	{
		UE_LOG(LogWalletAdapter, Error, TEXT("Authorization failed: %s"), *Exception->GetMessage());
		Failure.ExecuteIfBound(Exception->GetMessage());
		return;
	}

	check(JAuthFuture.IsValid());
	
	AsyncTask(ENamedThreads::AnyThread, [this, JAuthFuture, Success, Failure]
	{
		TSharedPtr<FThrowable> FutureGetException;		
		auto JAuthResult = JAuthFuture->Get(&FutureGetException);
		if (FutureGetException)
		{
			UE_LOG(LogWalletAdapter, Error, TEXT("Authorization failed: %s"), *FutureGetException->GetMessage());
			Failure.ExecuteIfBound(FutureGetException->GetMessage());
			return;
		}

		check(JAuthResult.IsValid());
		auto AuthResult = FAuthorizationResult::MakeFromExistingObject(JAuthResult->GetJObject());
		
		// SUCCESS
		UE_LOG(LogWalletAdapter, Log, TEXT("Authorized successfully: AuthToken = %s"), *AuthToken);
		AuthToken = AuthResult->GetAuthToken();
		PublicKey = AuthResult->GetPublicKey();
		AccountLabel = AuthResult->GetAccountLabel();
		WalletUriBase = AuthResult->GetWalletUriBase();
		
		Success.ExecuteIfBound(AuthToken);
	});;
#else
	Failure.ExecuteIfBound("Current platform is not supported");	
#endif
}

void UWalletAdapterClient::Reauthorize(FString IdentityUri, FString IconUri, FString IdentityName, FString AuthorizationToken, const FAuthSuccessDelegate& Success, const FFailureDelegate& Failure)
{
#if PLATFORM_ANDROID
	TSharedPtr<FThrowable> Exception;

	auto JAuthFuture = Client->Reauthorize(IdentityUri, IconUri, IdentityName, AuthorizationToken, Exception);
	if (Exception)
	{
		UE_LOG(LogWalletAdapter, Error, TEXT("Reauthorization failed: %s"), *Exception->GetMessage());
		Failure.ExecuteIfBound(Exception->GetMessage());
		return;
	}

	check(JAuthFuture.IsValid());

	AsyncTask(ENamedThreads::AnyThread, [this, JAuthFuture, Success, Failure]
	{
		TSharedPtr<FThrowable> FutureGetException;	
		auto JAuthResult = JAuthFuture->Get(&FutureGetException);
		if (FutureGetException)
		{
			UE_LOG(LogWalletAdapter, Error, TEXT("Reauthorization failed: %s"), *FutureGetException->GetMessage());
			Failure.ExecuteIfBound(FutureGetException->GetMessage());
			return;
		}

		check(JAuthResult.IsValid());
		auto AuthResult = FAuthorizationResult::MakeFromExistingObject(JAuthResult->GetJObject());
	
		// SUCCESS
		UE_LOG(LogWalletAdapter, Log, TEXT("Reauthorized successfully"));
		AuthToken = AuthResult->GetAuthToken();
		PublicKey = AuthResult->GetPublicKey();
		AccountLabel = AuthResult->GetAccountLabel();
		WalletUriBase = AuthResult->GetWalletUriBase();
			
		Success.ExecuteIfBound(AuthToken);
	});
#else
	Failure.ExecuteIfBound("Current platform is not supported");	
#endif
}

void UWalletAdapterClient::Deauthorize(const FString& AuthorizationToken, const FSuccessDelegate& Success, const FFailureDelegate& Failure)
{
#if PLATFORM_ANDROID
	TSharedPtr<FThrowable> Exception;

	auto JDeauthFuture = Client->Deauthorize(AuthorizationToken, Exception);
	if (Exception)
	{
		UE_LOG(LogWalletAdapter, Error, TEXT("Deauthorization failed: %s"), *Exception->GetMessage());
		Failure.ExecuteIfBound(Exception->GetMessage());
		return;
	}

	check(JDeauthFuture.IsValid());
	
	AsyncTask(ENamedThreads::AnyThread, [this, JDeauthFuture, Success, Failure]
	{
		TSharedPtr<FThrowable> FutureGetException;	
		JDeauthFuture->Get(&FutureGetException);
		if (FutureGetException)
		{
			UE_LOG(LogWalletAdapter, Error, TEXT("Deauthorization failed: %s"), *FutureGetException->GetMessage());
			Failure.ExecuteIfBound(FutureGetException->GetMessage());
			return;
		}	
		
		// SUCCESS
		UE_LOG(LogWalletAdapter, Log, TEXT("Deauthorized successfully"));
		AuthToken.Empty();
		Success.ExecuteIfBound();
	});
#else
	Failure.ExecuteIfBound("Current platform is not supported");	
#endif
}

void UWalletAdapterClient::SignTransactions(const TArray<FSolanaTransaction>& Transactions, const FSignSuccessDelegate& Success, const FFailureDelegate& Failure)
{
#if PLATFORM_ANDROID
	TSharedPtr<FThrowable> Exception;

	TArray<TArray<uint8>> RawTransactions;
	for (const auto& [Data] : Transactions)
		RawTransactions.Add(Data);

	auto JSignFuture = Client->SignTransactions(RawTransactions, Exception);
	if (Exception)
	{
		UE_LOG(LogWalletAdapter, Error, TEXT("Failed to sign %d transaction(s): %s"), Transactions.Num(), *Exception->GetMessage());
		Failure.ExecuteIfBound(Exception->GetMessage());
		return;
	}

	check(JSignFuture.IsValid());
	
	AsyncTask(ENamedThreads::AnyThread, [JSignFuture, Transactions, Success, Failure]
	{
		TSharedPtr<FThrowable> FutureGetException;
		
		auto JSignResult = JSignFuture->Get(&FutureGetException);
		if (FutureGetException)
		{
			UE_LOG(LogWalletAdapter, Error, TEXT("Failed to sign %d transaction(s): %s"), Transactions.Num(), *FutureGetException->GetMessage());
			Failure.ExecuteIfBound(FutureGetException->GetMessage());
			return;
		}

		check(JSignResult.IsValid());
		auto SignResult = FSignPayloadsResult::MakeFromExistingObject(JSignResult->GetJObject());
		
		// SUCCESS
		UE_LOG(LogWalletAdapter, Log, TEXT("Signed %d transaction(s)"), Transactions.Num());
		
		TArray<TArray<uint8>> SignedPayloads = SignResult->GetSignedPayloads();
		TArray<FSolanaTransaction> SignedTransactions;
		for (const TArray<uint8>& BytesArray : SignedPayloads)
			SignedTransactions.Add(FSolanaTransaction(BytesArray));
		Success.ExecuteIfBound(SignedTransactions);
	});
#else
	Failure.ExecuteIfBound("Current platform is not supported");	
#endif
}

void UWalletAdapterClient::SignAndSendTransactions(const TArray<FSolanaTransaction>& Transactions, int32 MinContextSlot, const FSignSuccessDelegate& Success, const FFailureDelegate& Failure)
{
#if PLATFORM_ANDROID
	TSharedPtr<FThrowable> Exception;

	TArray<TArray<uint8>> RawTransactions;
	for (const auto& [Data] : Transactions)
		RawTransactions.Add(Data);

	auto JSignAndSendFuture = Client->SignAndSendTransactions(RawTransactions, MinContextSlot ? &MinContextSlot : nullptr, Exception);
	if (Exception)
	{
		UE_LOG(LogWalletAdapter, Error, TEXT("Failed to sign and send %d transaction(s): %s"), Transactions.Num(), *Exception->GetMessage());
		Failure.ExecuteIfBound(Exception->GetMessage());
		return;
	}
	
	check(JSignAndSendFuture.IsValid());

	AsyncTask(ENamedThreads::AnyThread, [JSignAndSendFuture, Transactions, Success, Failure]
	{
		TSharedPtr<FThrowable> FutureGetException;
		auto JSignAndSendResult = JSignAndSendFuture->Get(&FutureGetException);
		if (FutureGetException)
		{
			UE_LOG(LogWalletAdapter, Error, TEXT("Failed to sign and send %d transaction(s): %s"), Transactions.Num(), *FutureGetException->GetMessage());
			Failure.ExecuteIfBound(FutureGetException->GetMessage());
			return;
		}

		check(JSignAndSendResult.IsValid());
		auto SignAndSendResult = FSignAndSendTransactionsResult::MakeFromExistingObject(JSignAndSendResult->GetJObject());
		
		// SUCCESS
		UE_LOG(LogWalletAdapter, Log, TEXT("Signed and sent %d transaction(s)"), Transactions.Num());
		
		TArray<TArray<uint8>> Signatures = SignAndSendResult->GetSignatures();
		TArray<FSolanaTransaction> SignedTransactions;
		for (const TArray<uint8>& BytesArray : Signatures)
			SignedTransactions.Add(FSolanaTransaction(BytesArray));
		
		Success.ExecuteIfBound(SignedTransactions);
	});
#else
	Failure.ExecuteIfBound("Current platform is not supported");	
#endif
}

void UWalletAdapterClient::K2_Authorize(FString IdentityUri, FString IconUri, FString IdentityName, FString Cluster, const FAuthSuccessDynDelegate& Success, const FFailureDynDelegate& Failure)
{
	Authorize(IdentityUri, IconUri, IdentityName, Cluster,
		FAuthSuccessDelegate::CreateLambda([Success](const FString& Token)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Success, Token]
			   {			
				   Success.ExecuteIfBound(Token);
			   });
			}
			else
			{
				Success.ExecuteIfBound(Token);
			}
		}),
		FFailureDelegate::CreateLambda([Failure](const FString& ErrorMessage)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Failure, ErrorMessage]
				{			
					Failure.ExecuteIfBound(ErrorMessage);
				});
			}
			else
			{
				Failure.ExecuteIfBound(ErrorMessage);
			}			
		}));
}

void UWalletAdapterClient::K2_Reauthorize(FString IdentityUri, FString IconUri, FString IdentityName, FString AuthorizationToken, const FAuthSuccessDynDelegate& Success, const FFailureDynDelegate& Failure)
{
	Reauthorize(IdentityUri, IconUri, IdentityName, AuthorizationToken,
		FAuthSuccessDelegate::CreateLambda([Success](const FString& Token)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Success, Token]
			   {
				   Success.ExecuteIfBound(Token);
			   });
			}
			else
			{
				Success.ExecuteIfBound(Token);
			}
		}),
		FFailureDelegate::CreateLambda([Failure](const FString& ErrorMessage)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Failure, ErrorMessage]
				{			
					Failure.ExecuteIfBound(ErrorMessage);
				});
			}
			else
			{
				Failure.ExecuteIfBound(ErrorMessage);
			}
		}));	
}

void UWalletAdapterClient::K2_Deauthorize(FString AuthorizationToken, const FSuccessDynDelegate& Success, const FFailureDynDelegate& Failure)
{
	Deauthorize(AuthorizationToken,
		FSuccessDelegate::CreateLambda([Success]()
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Success]
			   {
				   Success.ExecuteIfBound();
			   });
			}
			else
			{
				Success.ExecuteIfBound();
			}
		}),
		FFailureDelegate::CreateLambda([Failure](const FString& ErrorMessage)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Failure, ErrorMessage]
				{
				   Failure.ExecuteIfBound(ErrorMessage);
				});
			}
			else
			{
				Failure.ExecuteIfBound(ErrorMessage);
			}
		}));	
}

void UWalletAdapterClient::K2_SignTransactions(const TArray<FSolanaTransaction>& Transactions, const FSignSuccessDynDelegate& Success, const FFailureDynDelegate& Failure)
{
	SignTransactions(Transactions,
		FSignSuccessDelegate::CreateLambda([Success](const TArray<FSolanaTransaction>& Transactions)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Success, Transactions]
				{			
					Success.ExecuteIfBound(Transactions);
				});
			}
			else
			{
				Success.ExecuteIfBound(Transactions);
			}
		}),
		FFailureDelegate::CreateLambda([Failure](const FString& ErrorMessage)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Failure, ErrorMessage]
				{
					Failure.ExecuteIfBound(ErrorMessage);
				});
			}
			else
			{
				Failure.ExecuteIfBound(ErrorMessage);
			}
		}));
}

void UWalletAdapterClient::K2_SignAndSendTransactions(const TArray<FSolanaTransaction>& Transactions, int32 MinContextSlot, const FSignSuccessDynDelegate& Success, const FFailureDynDelegate& Failure)
{
	SignAndSendTransactions(Transactions, MinContextSlot,
		FSignSuccessDelegate::CreateLambda([Success](const TArray<FSolanaTransaction>& Transactions)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Success, Transactions]
				{
					Success.ExecuteIfBound(Transactions);
				});
			}
			else
			{
				Success.ExecuteIfBound(Transactions);
			}
		}),
		FFailureDelegate::CreateLambda([Failure](const FString& ErrorMessage)
		{
			if (!IsInGameThread())
			{
				AsyncTask(ENamedThreads::GameThread, [Failure, ErrorMessage]
				{			
					Failure.ExecuteIfBound(ErrorMessage);
				});
			}
			else
			{
				Failure.ExecuteIfBound(ErrorMessage);
			}
		}));
}
