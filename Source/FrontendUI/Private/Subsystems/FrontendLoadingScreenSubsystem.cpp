// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FrontendLoadingScreenSubsystem.h"

#include "FrontendDebugHelper.h"

bool UFrontendLoadingScreenSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
    {
        TArray<UClass*> FoundClasses;
        GetDerivedClasses(GetClass(),FoundClasses);

        return FoundClasses.IsEmpty();
    }

    return false;
}

void UFrontendLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this,&ThisClass::OnMapPreLoaded);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this,&ThisClass::OnMapPostLoaded);
}

void UFrontendLoadingScreenSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PreLoadMapWithContext.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
}

UWorld *UFrontendLoadingScreenSubsystem::GetTickableGameObjectWorld() const
{
    if (UGameInstance* OwningGameInstance = GetGameInstance())
    {
        return OwningGameInstance->GetWorld();
    }

    return nullptr;
}

void UFrontendLoadingScreenSubsystem::Tick(float DeltaTime)
{
    Debug::Print(TEXT("Ticking"));
    TryUpdateLoadingScreen();
}

ETickableTickType UFrontendLoadingScreenSubsystem::GetTickableTickType() const
{
    if (IsTemplate())
    {
        return ETickableTickType::Never;
    }

    return ETickableTickType::Conditional;
}

bool UFrontendLoadingScreenSubsystem::IsTickable() const
{
    return GetGameInstance() && GetGameInstance()->GetGameViewportClient();
}

TStatId UFrontendLoadingScreenSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UFrontendLoadingScreenSubsystem,STATGROUP_Tickables);
}

void UFrontendLoadingScreenSubsystem::OnMapPreLoaded(const FWorldContext &WorldContext, const FString &MapName)
{
    Debug::Print(TEXT("On Map Pre Loaded"));
    if (WorldContext.OwningGameInstance != GetGameInstance())
    {
        return;
    }

    SetTickableTickType(ETickableTickType::Conditional);

    bIsCurrentlyLoadingMap = true;

    TryUpdateLoadingScreen();
}

void UFrontendLoadingScreenSubsystem::OnMapPostLoaded(UWorld *LoadedWorld)
{
    Debug::Print(TEXT("On Map Post Loaded"));
    if (LoadedWorld && LoadedWorld->GetGameInstance() == GetGameInstance())
    {
        bIsCurrentlyLoadingMap = false;
    }
}

void UFrontendLoadingScreenSubsystem::TryUpdateLoadingScreen()
{
    //Check if there's any start up loading screen that's currently active


    //Check if we should show the loading screen
    if (true)
    {
        //Try display the loading screen here
    }
    else
    {
        //Try remove the current active loading screen

        //Modify the loading is completed

        //Disable the ticking
        SetTickableTickType(ETickableTickType::Never);
    }
}
