/**
 * @file MainPage.xaml.cpp
 * @brief Main Page Code-Behind for WinRT Frontend
 * 
 * Implements basic VM management UI for UWP / Windows Mobile.
 * Uses the IVBoxFrontend API for all VM operations.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#include "pch.h"
#include "MainPage.xaml.h"

#include <IVBoxFrontend.h>

using namespace LibreVMM::WinRT;

// VM descriptor for display
ref class VMDisplayItem
{
public:
    Platform::String^ Name;
    Platform::String^ Id;
    Platform::String^ State;
    Platform::String^ OSType;
};

// Snapshot descriptor for display
ref class SnapshotDisplayItem
{
public:
    Platform::String^ Name;
    Platform::String^ Id;
    Platform::String^ TimeStamp;
};

MainPage::MainPage()
{
    InitializeComponent();
    
    // Load VM list on startup
    this->Loaded += ref new RoutedEventHandler(this, &MainPage::Page_Loaded);
}

void MainPage::Page_Loaded(Object^ sender, RoutedEventArgs^ e)
{
    RefreshVMList();
}

// Refresh the VM list
void MainPage::RefreshVMList()
{
    // Clear existing items
    VMListView->Items->Clear();
    
    // TODO: Get VMs from frontend
    // auto vms = GetFrontend()->listVMs();
    // for (const auto& vm : vms) {
    //     auto item = ref new VMDisplayItem();
    //     item->Name = ref new Platform::String(vm.name.c_str());
    //     item->Id = ref new Platform::String(vm.id.c_str());
    //     item->State = ref new Platform::String(vm.state.c_str());
    //     item->OSType = ref new Platform::String(vm.ostype.c_str());
    //     VMListView->Items->Append(item);
    // }
    
    // Update status
    VMCountText->Text = VMListView->Items->Size + " VMs";
    StatusText->Text = "Ready";
}

// Handle VM selection
void MainPage::VMListView_SelectionChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
    if (VMListView->SelectedItem == nullptr) {
        DetailsPanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
        return;
    }
    
    auto selectedVM = dynamic_cast<VMDisplayItem^>(VMListView->SelectedItem);
    if (selectedVM) {
        // Show details panel
        DetailsPanel->Visibility = Windows::UI::Xaml::Visibility::Visible;
        
        // Update UI with selected VM details
        VMNameText->Text = selectedVM->Name;
        VMStateText->Text = selectedVM->State;
        
        // TODO: Get full config from frontend
        // auto config = GetFrontend()->getVMConfig(vmId);
        
        // Update action buttons
        UpdateActionButtons(selectedVM->State);
    }
}

// Update action buttons based on VM state
void MainPage::UpdateActionButtons(Platform::String^ state)
{
    bool isRunning = (state == "Running");
    bool isPaused = (state == "Paused");
    bool isPoweredOff = (state == "Powered Off");
    
    StartButton->IsEnabled = isPoweredOff;
    PauseButton->IsEnabled = isRunning;
    StopButton->IsEnabled = isRunning || isPaused;
}

// Refresh button clicked
void MainPage::RefreshButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    RefreshVMList();
    StatusText->Text = "VM list refreshed";
}

// New VM button clicked
void MainPage::NewVMButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    // Show new VM dialog
    // TODO: Implement VM creation wizard
    StatusText->Text = "New VM dialog";
}

// Start button clicked
void MainPage::StartButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto selectedVM = dynamic_cast<VMDisplayItem^>(VMListView->SelectedItem);
    if (selectedVM) {
        StatusText->Text = "Starting " + selectedVM->Name + "...";
        
        // TODO: Start VM via frontend
        // GetFrontend()->startVM(vmId, "gui");
        
        RefreshVMList();
    }
}

// Pause button clicked
void MainPage::PauseButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto selectedVM = dynamic_cast<VMDisplayItem^>(VMListView->SelectedItem);
    if (selectedVM) {
        StatusText->Text = "Pausing " + selectedVM->Name + "...";
        
        // TODO: Pause VM via frontend
        // GetFrontend()->pauseVM(vmId);
        
        RefreshVMList();
    }
}

// Stop button clicked
void MainPage::StopButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto selectedVM = dynamic_cast<VMDisplayItem^>(VMListView->SelectedItem);
    if (selectedVM) {
        StatusText->Text = "Stopping " + selectedVM->Name + "...";
        
        // TODO: Stop VM via frontend
        // GetFrontend()->powerOffVM(vmId);
        
        RefreshVMList();
    }
}

// Settings button clicked
void MainPage::SettingsButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto selectedVM = dynamic_cast<VMDisplayItem^>(VMListView->SelectedItem);
    if (selectedVM) {
        StatusText->Text = "Opening settings for " + selectedVM->Name;
        
        // TODO: Open settings dialog
        // ShowDevicePanel(vmId);
    }
}

// Take snapshot button clicked
void MainPage::TakeSnapshotButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto selectedVM = dynamic_cast<VMDisplayItem^>(VMListView->SelectedItem);
    if (selectedVM) {
        StatusText->Text = "Taking snapshot...";
        
        // TODO: Take snapshot via frontend
        // GetFrontend()->takeSnapshot(vmId, "Snapshot", "");
        
        RefreshVMList();
    }
}
