// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#define SLIDER_TICK_COUNT 100
#define DETECT_WAIT_TIME 2500
#define PREVIEW_UPDATE_TIME 25
#define DEFAULT_HIGH_VALUE 100

// might have to change this setup for Wiimote
#define PROFILES_PATH "Profiles/"

#include <cstddef>
#include <string>
#include <vector>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/eventfilter.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/timer.h>

#include "InputCommon/ControllerEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/ControllerInterface/Device.h"

class DolphinSlider;
class InputConfig;
class wxComboBox;
class wxListBox;
class wxStaticBitmap;
class wxStaticText;
class wxTextCtrl;

class PadSetting
{
protected:
  PadSetting(wxControl* const _control) : wxcontrol(_control) { wxcontrol->SetClientData(this); }
public:
  virtual void UpdateGUI() = 0;
  virtual void UpdateValue() = 0;

  virtual ~PadSetting() {}
  wxControl* const wxcontrol;
};

class PadSettingExtension : public PadSetting
{
public:
  PadSettingExtension(wxWindow* const parent, ControllerEmu::Extension* const ext);
  void UpdateGUI() override;
  void UpdateValue() override;

  ControllerEmu::Extension* const extension;
};

class PadSettingSpin : public PadSetting
{
public:
  PadSettingSpin(wxWindow* const parent,
                 ControllerEmu::ControlGroup::NumericSetting* const setting);
  void UpdateGUI() override;
  void UpdateValue() override;

  ControllerEmu::ControlGroup::NumericSetting* const setting;
};

class PadSettingCheckBox : public PadSetting
{
public:
  PadSettingCheckBox(wxWindow* const parent,
                     ControllerEmu::ControlGroup::BooleanSetting* const setting);
  void UpdateGUI() override;
  void UpdateValue() override;

  ControllerEmu::ControlGroup::BooleanSetting* const setting;
};

class InputEventFilter : public wxEventFilter
{
public:
  InputEventFilter() { wxEvtHandler::AddFilter(this); }
  ~InputEventFilter() { wxEvtHandler::RemoveFilter(this); }
  int FilterEvent(wxEvent& event) override;

  void BlockEvents(bool block) { m_block = block; }
private:
  static bool ShouldCatchEventType(wxEventType type)
  {
    return type == wxEVT_KEY_DOWN || type == wxEVT_KEY_UP || type == wxEVT_CHAR ||
           type == wxEVT_CHAR_HOOK || type == wxEVT_LEFT_DOWN || type == wxEVT_LEFT_UP ||
           type == wxEVT_MIDDLE_DOWN || type == wxEVT_MIDDLE_UP || type == wxEVT_RIGHT_DOWN ||
           type == wxEVT_RIGHT_UP;
  }

  bool m_block = false;
};

class InputConfigDialog;

class ControlDialog : public wxDialog
{
public:
  ControlDialog(InputConfigDialog* const parent, InputConfig& config,
                ControllerInterface::ControlReference* const ref);

  bool Validate() override;

  int GetRangeSliderValue() const;

  ControllerInterface::ControlReference* const control_reference;
  InputConfig& m_config;

private:
  wxStaticBoxSizer* CreateControlChooser(InputConfigDialog* parent);

  void UpdateGUI();
  void UpdateListContents();
  void SelectControl(const std::string& name);

  void DetectControl(wxCommandEvent& event);
  void ClearControl(wxCommandEvent& event);
  void SetDevice(wxCommandEvent& event);

  void SetSelectedControl(wxCommandEvent& event);
  void AppendControl(wxCommandEvent& event);
  void OnRangeSlide(wxScrollEvent&);
  void OnRangeSpin(wxSpinEvent&);
  void OnRangeThumbtrack(wxScrollEvent&);

  bool GetExpressionForSelectedControl(wxString& expr);

  InputConfigDialog* m_parent;
  wxComboBox* device_cbox;
  wxTextCtrl* textctrl;
  wxListBox* control_lbox;
  DolphinSlider* m_range_slider;
  wxSpinCtrl* m_range_spinner;
  wxStaticText* m_bound_label;
  wxStaticText* m_error_label;
  InputEventFilter m_event_filter;
  ciface::Core::DeviceQualifier m_devq;
};

class ExtensionButton : public wxButton
{
public:
  ExtensionButton(wxWindow* const parent, ControllerEmu::Extension* const ext)
      : wxButton(parent, wxID_ANY, _("Configure"), wxDefaultPosition), extension(ext)
  {
  }

  ControllerEmu::Extension* const extension;
};

class ControlButton : public wxButton
{
public:
  ControlButton(wxWindow* const parent, ControllerInterface::ControlReference* const _ref,
                const std::string& name, const unsigned int width, const std::string& label = {});

  ControllerInterface::ControlReference* const control_reference;
  const std::string m_name;

protected:
  wxSize DoGetBestSize() const override;

  int m_configured_width = wxDefaultCoord;
};

class ControlGroupBox : public wxStaticBoxSizer
{
public:
  ControlGroupBox(ControllerEmu::ControlGroup* const group, wxWindow* const parent,
                  InputConfigDialog* eventsink);
  ~ControlGroupBox();

  bool HasBitmapHeading() const
  {
    return control_group->type == GROUP_TYPE_STICK || control_group->type == GROUP_TYPE_TILT ||
           control_group->type == GROUP_TYPE_CURSOR || control_group->type == GROUP_TYPE_FORCE;
  }

  std::vector<PadSetting*> options;

  ControllerEmu::ControlGroup* const control_group;
  wxStaticBitmap* static_bitmap;
  std::vector<ControlButton*> control_buttons;
  double m_scale;
};

class InputConfigDialog : public wxDialog
{
public:
  InputConfigDialog(wxWindow* const parent, InputConfig& config, const wxString& name,
                    const int port_num = 0);
  virtual ~InputConfigDialog() = default;

  void OnClose(wxCloseEvent& event);
  void OnCloseButton(wxCommandEvent& event);

  void UpdateDeviceComboBox();
  void UpdateProfileComboBox();
  void UpdateTextureComboBoxes();

  void UpdateControlReferences();
  void UpdateBitmaps(wxTimerEvent&);

  void UpdateGUI();

  void RefreshDevices(wxCommandEvent& event);

  void LoadProfile(wxCommandEvent& event);
  void SaveProfile(wxCommandEvent& event);
  void DeleteProfile(wxCommandEvent& event);

  void ConfigControl(wxEvent& event);
  void ClearControl(wxEvent& event);
  void DetectControl(wxCommandEvent& event);

  void ConfigExtension(wxCommandEvent& event);

  void SetDevice(wxCommandEvent& event);

  void ClearAll(wxCommandEvent& event);
  void LoadDefaults(wxCommandEvent& event);

  void AdjustControlOption(wxCommandEvent& event);
  void EnablePadSetting(const std::string& group_name, const std::string& name, bool enabled);
  void EnableControlButton(const std::string& group_name, const std::string& name, bool enabled);
  void AdjustSetting(wxCommandEvent& event);
  void AdjustBooleanSetting(wxCommandEvent& event);

  void GetProfilePath(std::string& path);
  ControllerEmu* GetController() const;

  wxComboBox* profile_cbox = nullptr;
  wxComboBox* device_cbox = nullptr;
  wxComboBox* m_left_texture_box = nullptr, * m_right_texture_box = nullptr;

  std::vector<ControlGroupBox*> control_groups;
  std::vector<ControlButton*> control_buttons;

protected:
  wxBoxSizer* CreateDeviceChooserGroupBox();
  wxBoxSizer* CreaterResetGroupBox(wxOrientation orientation);
  wxBoxSizer* CreateProfileChooserGroupBox();
  wxSizer* CreateTextureChooserAndButtonBar();

  ControllerEmu* const controller;

  wxTimer m_update_timer;

private:
  InputConfig& m_config;
  int m_port_num;
  ControlDialog* m_control_dialog;
  InputEventFilter m_event_filter;

  bool DetectButton(ControlButton* button);
  bool m_iterate = false;
};