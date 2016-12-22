// Copyright 2016 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>
#include <vector>

#include <wx/event.h>
#include <wx/menu.h>

class MainMenuBar final : public wxMenuBar
{
public:
  class PopulatePerspectivesEvent;

  enum class MenuType
  {
    Regular,
    Debug
  };

  explicit MainMenuBar(MenuType type, long style = 0);

  void Refresh(bool erase_background, const wxRect* rect = nullptr) override;

  // private:
  void AddMenus();
  // public:
  void BindEvents();

  wxMenu* CreateFileMenu() const;
  wxMenu* CreateEmulationMenu() const;
  wxMenu* CreateMovieMenu() const;
  wxMenu* CreateOptionsMenu() const;
  wxMenu* CreateToolsMenu() const;
  wxMenu* CreateViewMenu() const;
  wxMenu* CreateJITMenu() const;
  wxMenu* CreateDebugMenu();
  wxMenu* CreateSymbolsMenu() const;
  wxMenu* CreateProfilerMenu() const;
  wxMenu* CreateHelpMenu() const;

  // private:
  void OnPopulatePerspectivesMenu(PopulatePerspectivesEvent&);

  void RefreshMenuLabels() const;
  void RefreshPlayMenuLabel() const;
  void RefreshSaveStateMenuLabels() const;
  void RefreshWiiSystemMenuLabel() const;

  void ClearSavedPerspectivesMenu() const;
  void PopulateSavedPerspectivesMenu(const std::vector<std::string>& label_names) const;
  void CheckPerspectiveWithID(int perspective_id) const;

  wxMenu* m_saved_perspectives_menu{};
  // public:
  MenuType m_type;  // {};
};

class MainMenuBar::PopulatePerspectivesEvent final : public wxEvent
{
public:
  PopulatePerspectivesEvent(int sender_id, wxEventType event_type,
                            std::vector<std::string> perspective_names, int active_perspective)
      : wxEvent{sender_id, event_type}, m_active_perspective{active_perspective}
  {
    m_perspective_names = std::move(perspective_names);
  }

  const std::vector<std::string>& PerspectiveNames() const { return m_perspective_names; }
  int ActivePerspective() const { return m_active_perspective; }
  wxEvent* Clone() const override { return new PopulatePerspectivesEvent(*this); }
private:
  std::vector<std::string> m_perspective_names;
  int m_active_perspective{};
};

wxDECLARE_EVENT(EVT_POPULATE_PERSPECTIVES_MENU, MainMenuBar::PopulatePerspectivesEvent);