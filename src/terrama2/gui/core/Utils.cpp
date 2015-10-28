/*
  Copyright (C) 2007 National Institute For Space Research (INPE) - Brazil.

  This file is part of TerraMA2 - a free and open source computational
  platform for analysis, monitoring, and alert of geo-environmental extremes.

  TerraMA2 is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  TerraMA2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with TerraMA2. See LICENSE. If not, write to
  TerraMA2 Team at <terrama2-team@dpi.inpe.br>.
*/

/*!
  \file terrama2/core/ApplicationController.cpp

  \brief The implementation of gui utils module

  \author Evandro Delatin
  \author Raphael Willian da Costa
*/

// TerraMA2
#include "Utils.hpp"
#include "../Exception.hpp"

// QT
#include <QDir>
#include <QMainWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>


void terrama2::gui::core::saveTerraMA2File(QMainWindow* appFocus, const QJsonObject& json)
{
  QString fileDestination = QFileDialog::getSaveFileName(appFocus, QObject::tr("TerraMA2 Export Data Provider"));
  if (fileDestination.isEmpty())
  {
    throw terrama2::gui::FileError() << terrama2::ErrorDescription(QObject::tr("Error while saving...."));
  }

  QDir dir(fileDestination);
  if (dir.exists())
    throw terrama2::gui::DirectoryError() << terrama2::ErrorDescription(QObject::tr("Invalid directory typed"));

  if (!fileDestination.endsWith(".terrama2"))
      fileDestination.append(".terrama2");

  QFile file(fileDestination);
  file.open(QIODevice::WriteOnly);
  QJsonDocument document = QJsonDocument(json);
  file.write(document.toJson());
  file.close();
}