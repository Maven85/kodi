/*
 *  Copyright (C) 2005-202 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "commons/ilog.h"

#include <memory>
#include <string>

class CFileItemList;

class CAppParams
{
public:
  CAppParams();
  virtual ~CAppParams() = default;

  int GetLogLevel() const { return m_logLevel; }
  void SetLogLevel(int logLevel) { m_logLevel = logLevel; }

  bool IsStartFullScreen() const { return m_startFullScreen; }
  void SetStartFullScreen(bool startFullScreen) { m_startFullScreen = startFullScreen; }

  bool IsStandAlone() const { return m_standAlone; }
  void SetStandAlone(bool standAlone) { m_standAlone = standAlone; }

  bool HasPlatformDirectories() const { return m_platformDirectories; }
  void SetPlatformDirectories(bool platformDirectories)
  {
    m_platformDirectories = platformDirectories;
  }

  bool IsTestMode() const { return m_testmode; }
  void SetTestMode(bool testMode) { m_testmode = testMode; }

  const std::string& GetSettingsFile() const { return m_settingsFile; }
  void SetSettingsFile(const std::string& settingsFile) { m_settingsFile = settingsFile; }

  const std::string& GetWindowing() const { return m_windowing; }
  void SetWindowing(const std::string& windowing) { m_windowing = windowing; }

  const std::string& GetLogTarget() const { return m_logTarget; }
  void SetLogTarget(const std::string& logTarget) { m_logTarget = logTarget; }

  CFileItemList& GetPlaylist() const { return *m_playlist; }

private:
  int m_logLevel{LOG_LEVEL_NORMAL};

  bool m_startFullScreen{false};
  bool m_standAlone{false};
  bool m_platformDirectories{true};
  bool m_testmode{false};

  std::string m_settingsFile;
  std::string m_windowing;
  std::string m_logTarget;

  std::unique_ptr<CFileItemList> m_playlist;
};
