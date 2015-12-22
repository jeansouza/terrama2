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
  \file terrama2/core/IntersectionDAO.hpp

  \brief Persistense layer for intersection information associated to a dataset.

  \author Paulo R. M. Oliveira
*/


//TerraMA2
#include "IntersectionDAO.hpp"
#include "Intersection.hpp"
#include "Exception.hpp"
#include "Utils.hpp"

// TerraLib
#include <terralib/dataaccess/datasource/DataSourceTransactor.h>

// STL
#include <map>

// Qt
#include <QObject>

//Boost
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

void
terrama2::core::IntersectionDAO::save(const Intersection& intersection, te::da::DataSourceTransactor& transactor)
{
  if(intersection.dataset() == 0)
    throw InvalidArgumentError() << ErrorDescription(QObject::tr("The dataset associated to the intersection must have a valid identifier (different than 0)."));

  std::vector<std::string> strs;

  auto attrMap = intersection.attributeMap();

  for(auto attrIt = attrMap.begin(); attrIt != attrMap.end(); attrIt++)
  {
    boost::split(strs, attrIt->first, boost::is_any_of("."));

    if(strs.size() != 2)
      throw InvalidArgumentError() << ErrorDescription(QObject::tr("Invalid table name."));

    std::string schemaName = strs[0];
    std::string tableName = strs[1];

    for(auto attr : attrIt->second)
    {
      boost::format query("INSERT INTO terrama2.intersection (dataset_id, schema_name, table_name, attribute) VALUES (%1%, '%2%', '%3%', '%4%')");
      query.bind_arg(1, intersection.dataset());

      // Schema name
      query.bind_arg(2, schemaName);

      // Table name
      query.bind_arg(3, tableName);

      // Attribute
      query.bind_arg(4, attr);

      try
      {
        transactor.execute(query.str());
      }
      catch(const std::exception& e)
      {
        throw DataAccessError() << ErrorDescription(e.what());
      }
      catch(...)
      {
        QString err_msg(QObject::tr("Unexpected error saving intersection information for dataset: %1"));

        err_msg = err_msg.arg(intersection.dataset());

        throw DataAccessError() << ErrorDescription(err_msg);
      }
    }

  }

  auto bandMap = intersection.bandMap();

  for(auto bandIt = bandMap.begin(); bandIt != bandMap.end(); bandIt++)
  {

    boost::format query("INSERT INTO terrama2.intersection (dataset_id, dataset_grid_id, band) VALUES(%1%, %2%, '%3%')");
    query.bind_arg(1, intersection.dataset());

    // dataset grid id
    query.bind_arg(2, bandIt->first);

    // band
    query.bind_arg(3, bandIt->second);

    try
    {
      transactor.execute(query.str());
    }
    catch(const std::exception& e)
    {
      throw DataAccessError() << ErrorDescription(e.what());
    }
    catch(...)
    {
      QString err_msg(QObject::tr("Unexpected error saving intersection information for dataset: %1"));

      err_msg = err_msg.arg(intersection.dataset());

      throw DataAccessError() << ErrorDescription(err_msg);
    }
  }

}

void
terrama2::core::IntersectionDAO::update(const Intersection& intersection, te::da::DataSourceTransactor& transactor)
{
  if(intersection.dataset() == 0)
    throw InvalidArgumentError() << ErrorDescription(QObject::tr("Can not update the intersction information with an invalid dataset identifier."));

  try
  {
    remove(intersection.dataset(), transactor);
    save(intersection, transactor);

  }
  catch(const terrama2::Exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    throw DataAccessError() << ErrorDescription(e.what());
  }
  catch(...)
  {
    throw DataAccessError() << ErrorDescription(QObject::tr("Could not update the intersection information."));
  }
}

void
terrama2::core::IntersectionDAO::remove(uint64_t datasetId, te::da::DataSourceTransactor& transactor)
{
  if(datasetId == 0)
    throw InvalidArgumentError() << ErrorDescription(QObject::tr("Can not remove the intersection information of a dataset with identifier: 0."));

  try
  {
    std::string sql = "DELETE FROM terrama2.intersection WHERE dataset_id = " + std::to_string(datasetId);
    transactor.execute(sql);
  }
  catch(...)
  {
    throw DataAccessError() << ErrorDescription(QObject::tr("Could not remove the intersection information."));
  }
}

void terrama2::core::IntersectionDAO::load(terrama2::core::DataSet& dataset, te::da::DataSourceTransactor& transactor)
{
  if(dataset.id() == 0)
    throw InvalidArgumentError() << ErrorDescription(QObject::tr("Can not load intersection information for an invalid dataset identifier: 0."));

  std::string sql("SELECT * FROM terrama2.intersection WHERE dataset_id = ");
  sql += std::to_string(dataset.id());

  try
  {
    std::auto_ptr<te::da::DataSet> intersectionResult = transactor.query(sql);

    Intersection intersection(dataset.id());

    std::map<std::string, std::vector<std::string> > attributeMap;
    std::map<uint64_t, std::string> bandMap;
    while(intersectionResult->moveNext())
    {
      if(!intersectionResult->isNull("dataset_grid_id"))
      {
        uint64_t datasetGridId = (uint64_t)intersectionResult->getInt32("dataset_grid_id");
        std::string band = intersectionResult->getString("band");
        bandMap[datasetGridId] = band;

      }

      std::string schemaName = intersectionResult->getString("schema_name");
      std::string tableName = intersectionResult->getString("table_name");
      std::string attribute = intersectionResult->getString("attribute");

      if(!schemaName.empty() and !tableName.empty() and !attribute.empty())
      {
        std::string compositeName = schemaName + "." + tableName;
        auto it = attributeMap.find(compositeName);
        if(it != attributeMap.end())
        {
          it->second.push_back(attribute);
        }
        else
        {
          std::vector<std::string> attrVec;
          attrVec.push_back(attribute);
          attributeMap[compositeName] = attrVec;
        }
      }
    }

    intersection.setAttributeMap(attributeMap);
    intersection.setBandMap(bandMap);

    dataset.setIntersection(intersection);
  }
  catch(const terrama2::Exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    throw DataAccessError() << ErrorDescription(e.what());
  }
  catch(...)
  {
    QString err_msg(QObject::tr("Unexpected error loading intersection information for dataset item: %1"));

    err_msg = err_msg.arg(dataset.id());

    throw DataAccessError() << ErrorDescription(err_msg);
  }

}


