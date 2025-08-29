// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Options/Widget_OptionsScreen.h"
#include "Input/CommonUIInputTypes.h"
#include "ICommonInputModule.h"
#include "Widgets/Options/OptionsDataRegistry.h"
#include "Widgets/Components/FrontendTabListWidgetBase.h"
#include "Widgets/Options/DataObjects/ListDataObject_Collection.h"
#include "Widgets/Components/FrontendCommonListView.h"
#include "FrontendSettings/FrontendGameUserSettings.h"
#include "Widgets/Options/ListEntries/Widget_ListEntry_Base.h"
#include "Widgets/Options/Widget_OptionsDetailsView.h"
#include "Subsystems/FrontendUISubsystem.h"
#include "Widgets/Components/FrontendCommonButtonBase.h"

#include "FrontendDebugHelper.h"

void UWidget_OptionsScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!ResetAction.IsNull())
    {
        ResetActionHandle = RegisterUIActionBinding(
            FBindUIActionArgs(
                ResetAction,
                true,
                FSimpleDelegate::CreateUObject(this, &ThisClass::OnResetBoundActionTriggered)
            )
        );
    }

    RegisterUIActionBinding(
        FBindUIActionArgs(
            ICommonInputModule::GetSettings().GetDefaultBackAction(),
            true,
            FSimpleDelegate::CreateUObject(this, &ThisClass::OnBackBoundActionTriggered)
        )
    );

    TabListWidget_OptionsTabs->OnTabSelected.AddUniqueDynamic(this,&ThisClass::OnOptionsTabSelected);

    CommonListView_OptionsList->OnItemIsHoveredChanged().AddUObject(this,&ThisClass::OnListViewItemHovered);
    CommonListView_OptionsList->OnItemSelectionChanged().AddUObject(this,&ThisClass::OnListViewItemSelected);
}

void UWidget_OptionsScreen::NativeOnActivated()
{
    Super::NativeOnActivated();

    for (UListDataObject_Collection* TabCollection : GetOrCreateDataRegistry()->GetRegisteredOptionsTabCollections())
    {
        if (!TabCollection)
        {
            continue;
        }

        const FName TabID = TabCollection->GetDataID();

        if (TabListWidget_OptionsTabs->GetTabButtonBaseByID(TabID) != nullptr)
        {
            continue;
        }

        TabListWidget_OptionsTabs->RequestRegisterTab(TabID,TabCollection->GetDataDisplayName());
    }
}

void UWidget_OptionsScreen::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();

    UFrontendGameUserSettings::Get()->ApplySettings(true);
}

UOptionsDataRegistry *UWidget_OptionsScreen::GetOrCreateDataRegistry()
{
    if (!CreatedOwningDataRegistry)
    {
        CreatedOwningDataRegistry = NewObject<UOptionsDataRegistry>();
        CreatedOwningDataRegistry->InitOptionsDataRegistry(GetOwningLocalPlayer());
    }

    checkf(CreatedOwningDataRegistry,TEXT("Data registry for options screen is not valid"));

    return CreatedOwningDataRegistry;
}

void UWidget_OptionsScreen::OnResetBoundActionTriggered()
{
    Debug::Print(TEXT("Reset bound action triggered"));
    if (ResettableDataArray.IsEmpty())
    {
        return;
    }

    UCommonButtonBase* SelectedTabButton = TabListWidget_OptionsTabs->GetTabButtonBaseByID(TabListWidget_OptionsTabs->GetActiveTab());

    const FString SelectedTabButtonName = CastChecked<UFrontendCommonButtonBase>(SelectedTabButton)->GetButtonDisplayText().ToString();

    UFrontendUISubsystem::Get(this)->PushConfirmScreenToModalStackAsync(
        EConfirmScreenType::YesNo,
        FText::FromString(TEXT("Reset")),
        FText::FromString(TEXT("Are you sure you want to reset all the settings under the ") + SelectedTabButtonName + TEXT(" tab?")),
        [this](EConfirmScreenButtonType ClickedButtonType)
        {
            if (ClickedButtonType != EConfirmScreenButtonType::Confirmed)
            {
                return;
            }

            bIsResettingData = true;
            bool bHasDataFailedToReset = false;

            for (UListDataObject_Base* DataToReset : ResettableDataArray)
            {
                if (!DataToReset)
                {
                    continue;
                }

                if (DataToReset->TryResetBackToDefaultValue())
                {
                    Debug::Print(DataToReset->GetDataDisplayName().ToString() + TEXT(" was reset"));
                }
                else
                {
                    bHasDataFailedToReset = true;
                    Debug::Print(DataToReset->GetDataDisplayName().ToString() + TEXT(" failed to reset"));
                }
            }

            if (!bHasDataFailedToReset)
            {
                ResettableDataArray.Empty();
                RemoveActionBinding(ResetActionHandle);
            }

            bIsResettingData = false;
        }
    );
}

void UWidget_OptionsScreen::OnBackBoundActionTriggered()
{
    DeactivateWidget();
}

void UWidget_OptionsScreen::OnOptionsTabSelected(FName TabId)
{
    DetailsView_ListEntryInfo->ClearDetailsViewInfo();

    TArray<UListDataObject_Base*> FoundListSourceItems = GetOrCreateDataRegistry()->GetListSourceItemsBySelectedTabID(TabId);

    CommonListView_OptionsList->SetListItems(FoundListSourceItems);
    CommonListView_OptionsList->RequestRefresh();

    if (CommonListView_OptionsList->GetNumItems() != 0)
    {
        CommonListView_OptionsList->NavigateToIndex(0);
        CommonListView_OptionsList->SetSelectedIndex(0);
    }

    ResettableDataArray.Empty();

    for (UListDataObject_Base* FoundListSourceItem : FoundListSourceItems)
    {
        if(!FoundListSourceItem)
        {
            continue;
        }

        if (!FoundListSourceItem->OnListDataModified.IsBoundToObject(this))
        {
            FoundListSourceItem->OnListDataModified.AddUObject(this,&ThisClass::OnListViewListDataModified);
        }

        if (FoundListSourceItem->CanResetBackToDefaultValue())
        {
            ResettableDataArray.AddUnique(FoundListSourceItem);
        }
    }

    if (ResettableDataArray.IsEmpty())
    {
        RemoveActionBinding(ResetActionHandle);
    }
    else
    {
        if (!GetActionBindings().Contains(ResetActionHandle))
        {
            AddActionBinding(ResetActionHandle);
        }
    }
}

void UWidget_OptionsScreen::OnListViewItemHovered(UObject *InHoveredItem, bool bWasHovered)
{
    if (!InHoveredItem)
    {
        return;
    }

    const FString DebugString =
    CastChecked<UListDataObject_Base>(InHoveredItem)->GetDataDisplayName().ToString() +
    TEXT(" was ") +
    (bWasHovered ? TEXT("hovered") : TEXT("unhovered"));

    UWidget_ListEntry_Base* HoveredEntryWidget = CommonListView_OptionsList->GetEntryWidgetFromItem<UWidget_ListEntry_Base>(InHoveredItem);
    Debug::Print(DebugString);
    check(HoveredEntryWidget);

    HoveredEntryWidget->NativeOnListEntryWidgetHovered(bWasHovered);
}

void UWidget_OptionsScreen::OnListViewItemSelected(UObject *InSelectedItem)
{
    if (!InSelectedItem)
    {
        return;
    }

    const FString DebugString =
    CastChecked<UListDataObject_Base>(InSelectedItem)->GetDataDisplayName().ToString() +
    TEXT(" was selected");

    Debug::Print(DebugString);
}

void UWidget_OptionsScreen::OnListViewListDataModified(UListDataObject_Base *ModifiedData, EOptionsListDataModifyReason ModifyReason)
{
    if (!ModifiedData || bIsResettingData)
    {
        return;
    }

    if (ModifiedData->CanResetBackToDefaultValue())
    {
        ResettableDataArray.AddUnique(ModifiedData);

        if (!GetActionBindings().Contains(ResetActionHandle))
        {
            AddActionBinding(ResetActionHandle);
        }
    }
    else
    {
        if (ResettableDataArray.Contains(ModifiedData))
        {
            ResettableDataArray.Remove(ModifiedData);
        }
    }

    if (ResettableDataArray.IsEmpty())
    {
        RemoveActionBinding(ResetActionHandle);
    }
}
