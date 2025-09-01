// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/ListEntries/Widget_ListEntry_KeyRemap.h"
#include "Widgets/Options/DataObjects/ListDataObject_KeyRemap.h"
#include "Widgets/Components/FrontendCommonButtonBase.h"
#include "Subsystems/FrontendUISubsystem.h"
#include "FrontendGameplayTags.h"
#include "FrontendFunctionLibrary.h"
#include "Widgets/Options/Widget_KeyRemapScreen.h"

#include "FrontendDebugHelper.h"

void UWidget_ListEntry_KeyRemap::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    CommonButton_RemapKey->OnClicked().AddUObject(this,&ThisClass::OnRemapKeyButtonClicked);
    CommonButton_ResetKeyBinding->OnClicked().AddUObject(this,&ThisClass::OnResetKeyBindingButtonClicked);
}

void UWidget_ListEntry_KeyRemap::OnOwningListDataObjectSet(UListDataObject_Base *InOwningListDataObject) 
{
    Super::OnOwningListDataObjectSet(InOwningListDataObject);

    CachedOwningKeyRemapDataObject = CastChecked<UListDataObject_KeyRemap>(InOwningListDataObject);

    CommonButton_RemapKey->SetButtonDisplayImage(CachedOwningKeyRemapDataObject->GetIconFromCurrentKey());
}

void UWidget_ListEntry_KeyRemap::OnOwningListDataObjectModified(UListDataObject_Base *OwningModifiedData, EOptionsListDataModifyReason ModifyReason)
{
    if (CachedOwningKeyRemapDataObject)
    {
        CommonButton_RemapKey->SetButtonDisplayImage(CachedOwningKeyRemapDataObject->GetIconFromCurrentKey());
    }
}

void UWidget_ListEntry_KeyRemap::OnRemapKeyButtonClicked()
{
    Debug::Print(TEXT("Remap Key Button Clicked"));
    UFrontendUISubsystem::Get(this)->PushSoftWidgetToStackAsync(
        FrontendGameplayTags::Frontend_WidgetStack_Modal,
        UFrontendFunctionLibrary::GetFrontendSoftWidgetClassByTag(FrontendGameplayTags::Frontend_Widget_KeyRemapScreen),
        [this](EAsyncPushWidgetState PushState, UWidget_ActivatableBase* PushedWidget)
        {
            if (PushState == EAsyncPushWidgetState::OnCreatedBeforePush)
            {
                UWidget_KeyRemapScreen* CreatedKeyRemapScreen = CastChecked<UWidget_KeyRemapScreen>(PushedWidget);

                if (CachedOwningKeyRemapDataObject)
                {
                    CreatedKeyRemapScreen->SetDesiredInputTypeToFilter(CachedOwningKeyRemapDataObject->GetDesiredInputKeyType());
                }
            }
        }
    );
}

void UWidget_ListEntry_KeyRemap::OnResetKeyBindingButtonClicked()
{
    Debug::Print(TEXT("Remap Key Binding Button Clicked"));
}