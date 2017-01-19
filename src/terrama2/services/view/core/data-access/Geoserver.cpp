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
  \file terrama2/services/view/core/data-access/Geoserver.cpp

  \brief Communication class between View service and GeoServer

  \author Vinicius Campanha
*/


// TerraMA2
#include "Geoserver.hpp"
#include "Exception.hpp"
#include "../Utils.hpp"
#include "../DataManager.hpp"
#include "../serialization/Serialization.hpp"
#include "../../core/JSonUtils.hpp"
#include "../../../../impl/DataAccessorFile.hpp"
#include "../../../../impl/DataAccessorPostGIS.hpp"
#include "../../../../core/data-model/DataProvider.hpp"
#include "../../../../core/utility/Raii.hpp"
#include "../../../../core/utility/Logger.hpp"
#include "../../../../core/utility/DataAccessorFactory.hpp"


// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/ws/core/CurlWrapper.h>
#include <terralib/ws/ogc/wms/client/WMSClient.h>
#include <terralib/geometry/Envelope.h>
#include <terralib/fe/PropertyName.h>
#include <terralib/fe/Literal.h>
#include <terralib/fe/BinaryComparisonOp.h>
#include <terralib/fe/Filter.h>
#include <terralib/fe/Globals.h>

// Qt
#include <QTemporaryFile>
#include <QUrl>


terrama2::services::view::core::GeoServer::GeoServer(const te::core::URI uri)
  : MapsServer(uri)
{

}


const te::core::URI& terrama2::services::view::core::GeoServer::uri() const
{
  return uri_;
}


void terrama2::services::view::core::GeoServer::setWorkspace(const std::string& workspace)
{
  workspace_ = workspace;
}


const std::string& terrama2::services::view::core::GeoServer::getWorkspace(const std::string& name) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPost(uri_.uri() + "/rest/workspaces/" + name);

  if(!uriPost.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPost.uri()));
  }

  std::string responseWorkspace;

  // Register style
  cURLwrapper.get(uriPost, responseWorkspace);

  if(cURLwrapper.responseCode() == 404)
  {
    throw NotFoundGeoserverException() << ErrorDescription(QString::fromStdString(cURLwrapper.response()));
  }
  else if(cURLwrapper.responseCode() != 200)
  {
    QString errMsg = QObject::tr("Error at get Workspace. ");
    TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
  }

  return cURLwrapper.response();
}


void terrama2::services::view::core::GeoServer::registerWorkspace(const std::string& name)
{
  if(!name.empty())
  {
    workspace_ = name;
  }

  try
  {
    getWorkspace(workspace_);
  }
  catch(NotFoundGeoserverException /*e*/)
  {
    te::ws::core::CurlWrapper cURLwrapper;

    te::core::URI uriPost(uri_.uri() + "/rest/workspaces");

    if(!uriPost.isValid())
    {
      QString errMsg = QObject::tr("Invalid URI.");
      TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
      throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPost.uri()));
    }

    // Register style
    cURLwrapper.post(uriPost, "<workspace><name>" + workspace_ + "</name></workspace>", "Content-Type: text/xml");

    if(cURLwrapper.responseCode() != 201)
    {
      QString errMsg = QObject::tr("Error to register the workspace.");
      TERRAMA2_LOG_ERROR() << errMsg << cURLwrapper.response();
      throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
    }
  }
}


const std::string& terrama2::services::view::core::GeoServer::workspace() const
{
  return workspace_;
}

const std::string& terrama2::services::view::core::GeoServer::getDataStore(const std::string& name) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriGet(uri_.uri() + "/rest/workspaces/" + workspace_ + "/datastores/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(name), "", "/")).toStdString());

  if(!uriGet.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriGet.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriGet.uri()));
  }

  std::string responseDataStore;

  // Register style
  cURLwrapper.get(uriGet, responseDataStore);

  if(cURLwrapper.responseCode() == 404)
  {
    throw NotFoundGeoserverException() << ErrorDescription(QString::fromStdString(cURLwrapper.response()));
  }
  else if(cURLwrapper.responseCode() != 200)
  {
    QString errMsg = QObject::tr("Error at get Data Store. ");
    TERRAMA2_LOG_ERROR() << errMsg << uriGet.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
  }

  return cURLwrapper.response();
}


void terrama2::services::view::core::GeoServer::registerPostGisDataStore(const std::string& dataStoreName,
                                                                         const std::map<std::string, std::string> connInfo) const
{
  try
  {
    getDataStore(dataStoreName);
  }
  catch(NotFoundGeoserverException /*e*/)
  {
    te::ws::core::CurlWrapper cURLwrapper;

    te::core::URI uriPost(uri_.uri() + "/rest/workspaces/" + workspace_ + "/datastores");

    if(!uriPost.isValid())
    {
      QString errMsg = QObject::tr("Invalid URI.");
      TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
      throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPost.uri()));
    }

    std::string xml = "<dataStore><name>" + dataStoreName +"</name>" +
                      "<connectionParameters>" +
                      "<host>" + connInfo.at("PG_HOST") + "</host>" +
                      "<port>" + connInfo.at("PG_PORT") +"</port>" +
                      "<database>" + connInfo.at("PG_DB_NAME") + "</database>" +
                      "<user>" + connInfo.at("PG_USER") + "</user>" +
                      "<passwd>" + connInfo.at("PG_USER") + "</passwd>" +
                      "<dbtype>postgis</dbtype>" +
                      "</connectionParameters>" +
                      "</dataStore>";

    // Register data store
    cURLwrapper.post(uriPost, xml, "Content-Type: text/xml");
  }
}


const std::string& terrama2::services::view::core::GeoServer::getFeature(const std::string& dataStoreName,
                                                                         const std::string& name) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriGet(uri_.uri() + "/rest/workspaces/" + workspace_ + "/datastores/"
                       +  QString(QUrl::toPercentEncoding(QString::fromStdString(dataStoreName), "", "/")).toStdString()
                       + "/featuretypes/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(name), "", "/")).toStdString());

  if(!uriGet.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriGet.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriGet.uri()));
  }

  std::string responseDataStore;

  // Register style
  cURLwrapper.get(uriGet, responseDataStore);

  if(cURLwrapper.responseCode() == 404)
  {
    throw NotFoundGeoserverException() << ErrorDescription(QString::fromStdString(cURLwrapper.response()));
  }
  else if(cURLwrapper.responseCode() != 200)
  {
    QString errMsg = QObject::tr("Error at get Feature. ");
    TERRAMA2_LOG_ERROR() << errMsg << uriGet.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
  }

  return cURLwrapper.response();
}


void terrama2::services::view::core::GeoServer::registerPostgisTable(const std::string& dataStoreName,
                                                                     std::map<std::string, std::string> connInfo,
                                                                     const std::string& tableName,
                                                                     const std::string& title,
                                                                     const std::unique_ptr<te::da::DataSetType>& dataSetType,
                                                                     const std::string& timestampPropertyName,
                                                                     const std::string& sql) const
{
  try
  {
    getFeature(dataStoreName, tableName);

    deleteVectorLayer(dataStoreName, tableName, true);
  }
  catch(NotFoundGeoserverException /*e*/)
  {
    // Do nothing
  }

  std::string geomName;
  te::gm::GeomType geomType;
  std::string srid;

  if(dataSetType->hasGeom())
  {
    auto geomProperty = te::da::GetFirstGeomProperty(dataSetType.get());
    geomName = geomProperty->getName();
    geomType = geomProperty->getGeometryType();
    srid = std::to_string(geomProperty->getSRID());
  }

  registerPostGisDataStore(dataStoreName, connInfo);

  te::ws::core::CurlWrapper cURLwrapper;

  std::string xml = "<featureType>"
                    "<title>" + title + "</title>";

  xml += "<name>"+ tableName + "</name>";

  xml += "<enabled>true</enabled>";

  std::string metadataTime = "";
  std::string metadataSQL = "";

  if(!timestampPropertyName.empty())
  {
    metadataTime = "<entry key=\"time\">"
                   "<dimensionInfo>"
                   "<enabled>true</enabled>"
                   "<attribute>"+timestampPropertyName+"</attribute>"+
                   "<presentation>CONTINUOUS_INTERVAL</presentation>"
                   "<units>ISO8601</units>"
                   "<defaultValue>"
                   "<strategy>MAXIMUM</strategy>"
                   "</defaultValue>"
                   "</dimensionInfo>"
                   "</entry>"
                   "<entry key=\"cachingEnabled\">false</entry>";

  }

  if(!sql.empty())
  {
    metadataSQL = "<entry key=\"JDBC_VIRTUAL_TABLE\">"
                  "<virtualTable>"
                  "<name>"+title+"</name>" +
                  "<sql>"+sql+"</sql>" +
                  "<escapeSql>false</escapeSql>";

    if(!geomName.empty())
    {
      metadataSQL += "<geometry>"
                     "<name>"+geomName+"</name>" +
                     "<type>"+getGeomTypeString(geomType)+"</type>" +
                     "<srid>"+srid+"</srid>" +
                     "</geometry>";
    }

    metadataSQL += "</virtualTable>"
                   "</entry>";
  }

  if(!metadataTime.empty() || !metadataSQL.empty())
  {
    xml += "<metadata>" + metadataTime + metadataSQL + "</metadata>";
  }

  xml += "</featureType>";

  std::string uri = uri_.uri() + "/rest/workspaces/" + workspace_ + "/datastores/"
                    + QString(QUrl::toPercentEncoding(QString::fromStdString(dataStoreName), "", "/")).toStdString()
                    +"/featuretypes";

  te::core::URI uriPostLayer(uri);
  cURLwrapper.post(uriPostLayer, xml, "Content-Type: text/xml");

  if(cURLwrapper.responseCode() != 201)
  {
    QString errMsg = QObject::tr("Error at register PostGis Table. ");
    TERRAMA2_LOG_ERROR() << errMsg << uriPostLayer.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
  }
}


void terrama2::services::view::core::GeoServer::uploadZipVectorFiles(const std::string& dataStoreName,
                                                                     const std::string& shpZipFilePath,
                                                                     const std::string& extension) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/datastores/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(dataStoreName), "", "/")).toStdString()
                       + "/file." + extension + "?configure=all&update=append");

  if(!uriPut.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }
  // Upload Vector file
  cURLwrapper.putFile(uriPut, shpZipFilePath, "Content-type: application/zip");
}


void terrama2::services::view::core::GeoServer::registerVectorFile(const std::string& dataStoreName,
                                                                   const std::string& shpFilePath,
                                                                   const std::string& extension) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/datastores/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(dataStoreName), "", "/")).toStdString()
                       + "/external." + extension + "?configure=first&update=append");

  if(!uriPut.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }

  // Register Vector file
  cURLwrapper.customRequest(uriPut, "PUT", "file://" + shpFilePath);
}


void terrama2::services::view::core::GeoServer::registerVectorsFolder(const std::string &dataStoreName,
                                                                      const std::string &shpFolderPath,
                                                                      const std::string &extension) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/datastores/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(dataStoreName), "", "/")).toStdString()
                       + "/external." + extension + "?configure=all&update=append");

  if(!uriPut.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }

  // Register Vector file
  cURLwrapper.customRequest(uriPut, "PUT", "file://" + shpFolderPath);
}


void terrama2::services::view::core::GeoServer::uploadZipCoverageFile(const std::string& coverageStoreName,
                                                                      const std::string& coverageZipFilePath,
                                                                      const std::string& extension) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/coveragestores/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(coverageStoreName), "", "/")).toStdString()
                       + "/file." + extension + "?configure=all");

  if(!uriPut.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }
  // Upload Coverage file
  cURLwrapper.putFile(uriPut, coverageZipFilePath, "Content-type: application/zip");
}


void terrama2::services::view::core::GeoServer::registerCoverageFile(const std::string& coverageStoreName,
                                                                     const std::string& coverageFilePath,
                                                                     const std::string& coverageName,
                                                                     const std::string& extension,
                                                                     const std::string& styleName) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/coveragestores/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(coverageStoreName), "", "/")).toStdString()
                       + "/external." + extension + "?configure=first");

  if(!uriPut.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }
  // Upload Coverage file
  cURLwrapper.customRequest(uriPut, "PUT", "file://" + coverageFilePath);

  if(!styleName.empty())
  {
    te::core::URI layerStyle(uri_.uri() + "/rest/layers/" + coverageName + ".xml");

    cURLwrapper.customRequest(layerStyle, "PUT",
                              "<layer><defaultStyle><name>" + styleName + "</name><workspace>" + workspace_ + "</workspace></defaultStyle></layer>", "Content-Type: text/xml");
  }
}


void terrama2::services::view::core::GeoServer::registerMosaicCoverage(const std::string& coverageStoreName,
                                                                       const std::string& mosaicPath,
                                                                       const std::string& coverageName,
                                                                       const int srid,
                                                                       const std::string& style) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/coveragestores/"
                       + QString(QUrl::toPercentEncoding(QString::fromStdString(coverageStoreName), "", "/")).toStdString()
                       + "/external.imagemosaic?configure=all");

  if(!uriPut.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }

  std::string path = "file://" + mosaicPath;

  // Upload Coverage file
  cURLwrapper.customRequest(uriPut, "PUT", path, "Content-Type: text/plain");

  if(cURLwrapper.responseCode() != 201)
  {
    QString errMsg = QObject::tr(cURLwrapper.response().c_str());
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }

  te::core::URI uriPutUpdateCoverage(uri_.uri() + "/rest/workspaces/" + workspace_ + "/coveragestores/"
                                     + QString(QUrl::toPercentEncoding(QString::fromStdString(coverageStoreName), "", "/")).toStdString()
                                     + "/coverages/" + coverageName);

  std::string xml = "<coverage>"
                    "<enabled>true</enabled>"
                    "<srs>EPSG:"
                    + std::to_string(srid) +
                    "</srs>"
                    "<metadata>"
                    "<entry key=\"time\">"
                    "<dimensionInfo>"
                    "<enabled>true</enabled>"
                    "<presentation>CONTINUOUS_INTERVAL</presentation>"
                    "<units>ISO8601</units>"
                    "<defaultValue>"
                    "<strategy>MAXIMUM</strategy>"
                    "</defaultValue>"
                    "</dimensionInfo>"
                    "</entry>"
                    "</metadata>"
                    "</coverage>";

  cURLwrapper.customRequest(uriPutUpdateCoverage, "PUT", xml, "Content-Type: text/xml");

  if(cURLwrapper.responseCode() != 200)
  {
    QString errMsg = QObject::tr(cURLwrapper.response().c_str());
    TERRAMA2_LOG_ERROR() << errMsg << uriPutUpdateCoverage.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }

  if(!style.empty())
  {
    te::core::URI layerStyle(uri_.uri() + "/rest/layers/" + coverageName + ".xml");

    cURLwrapper.customRequest(layerStyle, "PUT",
                              "<layer><defaultStyle><name>" + style + "</name><workspace>" + workspace_ + "</workspace></defaultStyle></layer>", "Content-Type: text/xml");
  }

}


void terrama2::services::view::core::GeoServer::registerStyleFile(const std::string& name,
                                                                  const std::string& styleFilePath) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPost(uri_.uri() + "/rest/workspaces/" + workspace_ + "/styles");

  if(!uriPost.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPost.uri()));
  }

  // Register style
  cURLwrapper.post(uriPost, "<style><name>" + name + "</name><filename>" + name + ".sld</filename></style>", "Content-Type: text/xml");


  te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/styles/" + name +"?raw=true");

  if(!uriPut.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
  }

  // Upload Style file
  cURLwrapper.putFile(uriPut, styleFilePath, "Content-type: application/vnd.ogc.se+xml");
}


void terrama2::services::view::core::GeoServer::registerStyle(const std::string &name,
                                                              const std::unique_ptr<te::se::Style> &style) const
{
  QTemporaryFile file;
  if(!file.open())
  {
    QString errMsg = QObject::tr("Can't open the file.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw ViewGeoserverException() << ErrorDescription(errMsg);
  }

  std::string filePath = file.fileName().toStdString();

  Serialization::writeStyleGeoserverXML(style.get(), filePath);

  QByteArray content = file.readAll();
  if(content.isEmpty())
  {
    QString errMsg = QObject::tr("Can't read the SLD file.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw ViewGeoserverException() << ErrorDescription(errMsg);
  }

  registerStyle(name, QString(content).toStdString());
}


void terrama2::services::view::core::GeoServer::registerStyle(const std::string& name, const std::string &style) const
{
  std::string validName = QString(QUrl::toPercentEncoding(QString::fromStdString(name), "", "/")).toStdString();

  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriPost(uri_.uri() + "/rest/workspaces/" + workspace_ + "/styles?name="
                        + validName
                        + "&raw=true");

  if(!uriPost.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPost.uri()));
  }

  // Register style
  cURLwrapper.post(uriPost, style, "Content-Type: application/vnd.ogc.se+xml");

  if(cURLwrapper.responseCode() == 403)
  {
    te::core::URI uriPut(uri_.uri() + "/rest/workspaces/" + workspace_ + "/styles/" + validName +"?raw=true");

    if(!uriPut.isValid())
    {
      QString errMsg = QObject::tr("Invalid URI.");
      TERRAMA2_LOG_ERROR() << errMsg << uriPut.uri();
      throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriPut.uri()));
    }

    cURLwrapper.customRequest(uriPut, "PUT", style, "Content-Type: application/vnd.ogc.se+xml");

    if(cURLwrapper.responseCode() != 200)
    {
      QString errMsg = QObject::tr(cURLwrapper.response().c_str());
      TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
      throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
    }
  }
  else if(cURLwrapper.responseCode() != 201)
  {
    QString errMsg = QObject::tr(cURLwrapper.response().c_str());
    TERRAMA2_LOG_ERROR() << errMsg << uriPost.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
  }
}


void terrama2::services::view::core::GeoServer::registerStyle(const std::string& name,
                                                              const View::Legend& legend,
                                                              const std::unique_ptr<te::da::DataSetType>& dataSetType) const
{
  std::unique_ptr<te::se::Style> style(generateStyle(legend, dataSetType).release());

  registerStyle(name, style);
}

std::unique_ptr<te::se::Style> terrama2::services::view::core::GeoServer::generateStyle(const View::Legend& legend,
                                                                                        const std::unique_ptr<te::da::DataSetType>& dataSetType) const
{
  std::unique_ptr<te::se::Style> style;

  if(dataSetType->hasGeom())
  {
    style.reset(new te::se::FeatureTypeStyle());
  }
  else if(dataSetType->hasRaster())
  {
    style.reset(new te::se::CoverageStyle());
  }

  if(legend.ruleType == View::Legend::VALUE)
  {
    std::vector<te::se::Rule*> rules;
    te::se::Rule* ruleDefault;

    for(auto& legendRule : legend.rules)
    {
      te::se::Symbolizer* symbolizer(getSymbolizer(dataSetType, legendRule.color));

      te::se::Rule* rule = new te::se::Rule;
      rule->push_back(symbolizer);

      if(legendRule.isDefault)
      {
        ruleDefault = rule;
        continue;
      }

      te::fe::PropertyName* propertyName = new te::fe::PropertyName(legend.column);
      te::fe::Literal* value = new te::fe::Literal(legendRule.value);
      te::fe::BinaryComparisonOp* stateEqual = new te::fe::BinaryComparisonOp(te::fe::Globals::sm_propertyIsEqualTo, propertyName, value);

      te::fe::Filter* filter = new te::fe::Filter;
      filter->setOp(stateEqual);

      rule->setFilter(filter);

      rules.push_back(rule);
    }

    style->push_back(ruleDefault);

    for(auto& rule : rules)
    {
      style->push_back(rule);
    }
  }

  return style;
}

void terrama2::services::view::core::GeoServer::deleteWorkspace(bool recursive) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  std::string url = "/rest/workspaces/" + workspace_;

  if(recursive)
  {
    url += "?recurse=true";
  }

  te::core::URI uriDelete(uri_.uri() + url);

  if(!uriDelete.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriDelete.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriDelete.uri()));
  }

  cURLwrapper.customRequest(uriDelete, "delete");
}


void terrama2::services::view::core::GeoServer::deleteVectorLayer(const std::string& dataStoreName,
                                                                  const std::string &fileName,
                                                                  bool recursive) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  std::string url = "/rest/workspaces/" + workspace_ + "/datastores/"
                    + QString(QUrl::toPercentEncoding(QString::fromStdString(dataStoreName), "", "/")).toStdString()
                    + "/featuretypes/"
                    + QString(QUrl::toPercentEncoding(QString::fromStdString(fileName), "", "/")).toStdString();

  if(recursive)
  {
    url += "?recurse=true";
  }

  te::core::URI uriDelete(uri_.uri() + url);

  if(!uriDelete.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriDelete.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriDelete.uri()));
  }

  cURLwrapper.customRequest(uriDelete, "delete");
}


void terrama2::services::view::core::GeoServer::deleteCoverageLayer(const std::string& coverageStoreName,
                                                                    const std::string& fileName,
                                                                    bool recursive) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  std::string url = "/rest/workspaces/" + workspace_ + "/coveragestores/"
                    + QString(QUrl::toPercentEncoding(QString::fromStdString(coverageStoreName), "", "/")).toStdString()
                    + "/coverages/"
                    + QString(QUrl::toPercentEncoding(QString::fromStdString(fileName), "", "/")).toStdString();

  if(recursive)
  {
    url += "?recurse=true";
  }

  te::core::URI uriDelete(uri_.uri() + url);

  if(!uriDelete.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriDelete.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriDelete.uri()));
  }

  cURLwrapper.customRequest(uriDelete, "delete");
}


void terrama2::services::view::core::GeoServer::deleteStyle(const std::string& styleName) const
{
  te::ws::core::CurlWrapper cURLwrapper;

  te::core::URI uriDelete(uri_.uri() + "/rest/workspaces/" + workspace_ + "/styles/"
                          + QString(QUrl::toPercentEncoding(QString::fromStdString(styleName), "", "/")).toStdString() + "?purge=true");

  if(!uriDelete.isValid())
  {
    QString errMsg = QObject::tr("Invalid URI.");
    TERRAMA2_LOG_ERROR() << errMsg << uriDelete.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(uriDelete.uri()));
  }

  cURLwrapper.customRequest(uriDelete, "delete");

  if(cURLwrapper.responseCode() == 404)
  {
    throw NotFoundGeoserverException() << ErrorDescription(QString::fromStdString(cURLwrapper.response()));
  }
  else if(cURLwrapper.responseCode() != 200)
  {
    QString errMsg = QObject::tr("Error at delete style. ");
    TERRAMA2_LOG_ERROR() << errMsg << uriDelete.uri();
    throw ViewGeoserverException() << ErrorDescription(errMsg + QString::fromStdString(cURLwrapper.response()));
  }
}


void terrama2::services::view::core::GeoServer::getMapWMS(const std::string& savePath,
                                                          const std::string& fileName,
                                                          const std::list<std::pair<std::string, std::string>> layersAndStyles,
                                                          const te::gm::Envelope env,
                                                          const uint32_t width,
                                                          const uint32_t height,
                                                          const uint32_t srid,
                                                          const std::string& format) const
{
  if(layersAndStyles.empty())
  {
    QString errMsg = QObject::tr("No layers defined.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw ViewGeoserverException() << ErrorDescription(errMsg);
  }

  std::string version = "1.1.0";

  te::ws::ogc::WMSClient wms(savePath, uri_.uri(), version);

  std::string request = (uri_.uri() + "/wms?service=WMS" +
                         "&version=" + version +
                         "&request=GetMap");

  std::string layers = "&layers=";
  std::string styles = "&styles=";

  for(auto it = layersAndStyles.begin(); it != layersAndStyles.end(); ++it)
  {
    if(it == layersAndStyles.begin())
    {
      layers += it->first;
      styles += it->second;
    }
    else
    {
      layers += "," + it->first;
      styles += "," + it->second;
    }
  }

  request += layers;
  request += styles;
  request += "&bbox=" + std::to_string(env.getLowerLeftX()) + "," + std::to_string(env.getLowerLeftY()) + "," + std::to_string(env.getUpperRightX()) + "," + std::to_string(env.getUpperRightY());
  request += "&width=" + std::to_string(width);
  request += "&height=" + std::to_string(height);
  request += "&srs=EPSG:" + std::to_string(srid);
  request += "&format=" + format;

  te::core::URI uriRequest(request);

  wms.makeFileRequest(uriRequest.uri(), fileName);
}

QJsonObject terrama2::services::view::core::GeoServer::generateLayers(const ViewPtr viewPtr,
                                                                      const std::unordered_map< terrama2::core::DataSeriesPtr, terrama2::core::DataProviderPtr >& dataSeriesProviders,
                                                                      const std::shared_ptr<DataManager> dataManager,
                                                                      std::shared_ptr<ViewLogger> logger,
                                                                      const RegisterId logId)
{
  QJsonObject jsonAnswer;

  for(auto dataSeriesProvider : dataSeriesProviders)
  {
    terrama2::core::DataSeriesPtr inputDataSeries = dataSeriesProvider.first;
    terrama2::core::DataProviderPtr inputDataProvider = dataSeriesProvider.second;

    // Check if the view can be done by the maps server
    DataProviderType dataProviderType = inputDataProvider->dataProviderType;

    if(dataProviderType != "POSTGIS" && dataProviderType != "FILE")
    {
      TERRAMA2_LOG_ERROR() << QObject::tr("Data provider not supported: %1.").arg(dataProviderType.c_str());
      continue;
    }

    DataFormat dataFormat = inputDataSeries->semantics.dataFormat;

    if(dataFormat != "OGR" && dataFormat != "POSTGIS" && dataFormat != "GEOTIFF")
    {
      TERRAMA2_LOG_WARNING() << QObject::tr("Data format not supported in the maps server: %1.").arg(dataFormat.c_str());
    }

    registerWorkspace();

    // Register style
    std::string styleName = "";
    auto itStyle = viewPtr->stylesPerDataSeries.find(inputDataSeries->id);

    if(itStyle != viewPtr->stylesPerDataSeries.end())
    {
      styleName = viewPtr->viewName + "style" + std::to_string(inputDataSeries->id);
      registerStyle(styleName, itStyle->second);
    }

    QJsonArray layersArray;

    terrama2::core::DataAccessorPtr dataAccessor =
        terrama2::core::DataAccessorFactory::getInstance().make(inputDataProvider, inputDataSeries);

    terrama2::core::Filter filter;

    auto it = viewPtr->filtersPerDataSeries.find(inputDataSeries->id);

    if(it != viewPtr->filtersPerDataSeries.end())
    {
      filter = terrama2::core::Filter(it->second);
    }

    auto remover = std::make_shared<terrama2::core::FileRemover>();

    if(inputDataSeries->datasetList.empty())
    {
      logger->log(ViewLogger::WARNING_MESSAGE, "No data to register.", logId);
      TERRAMA2_LOG_WARNING() << QObject::tr("No data to register in maps server.");
      continue;
    }

    // DataSetType model to use in style creation
    std::unique_ptr< te::da::DataSetType > modelDataSetType;

    if(dataProviderType == "FILE")
    {
      std::shared_ptr< terrama2::core::DataAccessorFile > dataAccessorFile =
          std::dynamic_pointer_cast<terrama2::core::DataAccessorFile>(dataAccessor);

      for(auto& dataset : inputDataSeries->datasetList)
      {
        std::string timezone;
        try
        {
          timezone = dataAccessorFile->getTimeZone(dataset);
        }
        catch(const terrama2::core::UndefinedTagException& /*e*/)
        {
          //if timezone is not defined
          timezone = "UTC+00";
        }

        // Get the list of layers to register
        auto fileInfoList = dataAccessorFile->getFilesList(inputDataProvider->uri,
                                                           dataAccessorFile->getMask(dataset),
                                                           filter,
                                                           timezone,
                                                           dataset,
                                                           remover);

        if(fileInfoList.empty())
        {
          QString errorMsg = QString("No data in data series %1.").arg(inputDataSeries->id);
          logger->log(ViewLogger::WARNING_MESSAGE, errorMsg.toStdString(), logId);
          TERRAMA2_LOG_WARNING() << QObject::tr(errorMsg.toStdString().c_str());
          continue;
        }

        for(auto& fileInfo : fileInfoList)
        {
          if(!modelDataSetType)
          {
            std::shared_ptr<te::da::DataSource> datasource(te::da::DataSourceFactory::make("OGR",
                                                                                           "file://"+fileInfo.absolutePath().toStdString()));

            terrama2::core::OpenClose<std::shared_ptr<te::da::DataSource>> openClose(datasource);

            if(!datasource->isOpened())
            {
              QString errMsg = QObject::tr("DataProvider could not be opened.");
              logger->log(ViewLogger::ERROR_MESSAGE, errMsg.toStdString(), logId);
              TERRAMA2_LOG_ERROR() << errMsg;
              continue;
            }

            modelDataSetType = std::move(datasource->getDataSetType(fileInfo.baseName().toStdString()));
          }

          if(dataFormat == "OGR")
          {
            registerVectorFile(viewPtr->viewName + std::to_string(inputDataSeries->id) + "datastore",
                               fileInfo.absoluteFilePath().toStdString(),
                               fileInfo.completeSuffix().toStdString());
          }
          else if(dataFormat == "GEOTIFF")
          {
            registerCoverageFile(fileInfo.fileName().toStdString() ,
                                 fileInfo.absoluteFilePath().toStdString(),
                                 fileInfo.completeBaseName().toStdString(),
                                 "geotiff",
                                 styleName);
          }

          QJsonObject layer;
          layer.insert("layer", fileInfo.completeBaseName());
          layersArray.push_back(layer);
        }
      }
    }
    else if(dataProviderType == "POSTGIS")
    {
      QUrl url(inputDataProvider->uri.c_str());
      std::map<std::string, std::string> connInfo
      {
        {"PG_HOST", url.host().toStdString()},
        {"PG_PORT", std::to_string(url.port())},
        {"PG_USER", url.userName().toStdString()},
        {"PG_PASSWORD", url.password().toStdString()},
        {"PG_DB_NAME", url.path().section("/", 1, 1).toStdString()},
        {"PG_CONNECT_TIMEOUT", "4"},
        {"PG_CLIENT_ENCODING", "UTF-8"}
      };

      std::shared_ptr< terrama2::core::DataAccessorPostGIS > dataAccessorPostGis =
          std::dynamic_pointer_cast<terrama2::core::DataAccessorPostGIS>(dataAccessor);

      for(auto& dataset : inputDataSeries->datasetList)
      {
        std::string tableName = dataAccessorPostGis->getDataSetTableName(dataset);
        std::string layerName = viewPtr->viewName + "_"  + tableName;
        std::string timestampPropertyName;
        std::string joinSQL;

        try
        {
          timestampPropertyName = dataAccessorPostGis->getTimestampPropertyName(dataset);
        }
        catch (...)
        {

        }

        std::shared_ptr<te::da::DataSource> datasource(te::da::DataSourceFactory::make("POSTGIS",
                                                                                       inputDataProvider->uri));

        terrama2::core::OpenClose<std::shared_ptr<te::da::DataSource>> openClose(datasource);

        if(!datasource->isOpened())
        {
          QString errMsg = QObject::tr("DataProvider could not be opened.");
          logger->log(ViewLogger::ERROR_MESSAGE, errMsg.toStdString(), logId);
          TERRAMA2_LOG_ERROR() << errMsg;
          continue;
        }

        if(inputDataSeries->semantics.dataSeriesType != terrama2::core::DataSeriesType::ANALYSIS_MONITORED_OBJECT)
        {
          if(!modelDataSetType)
          {
            modelDataSetType = std::move(datasource->getDataSetType(tableName));
          }
        }
        else
        {
          const auto& id = dataset->format.find("monitored_object_id");

          if(id == dataset->format.end())
          {
            logger->log(ViewLogger::ERROR_MESSAGE, "Data to join not informed.", logId);
            TERRAMA2_LOG_ERROR() << QObject::tr("Cannot join data from a different DB source!");
            continue;
          }

          terrama2::core::DataSeriesPtr monitoredObjectDataSeries = dataManager->findDataSeries(std::stoi(id->second));
          terrama2::core::DataProviderPtr monitoredObjectProvider = dataManager->findDataProvider(monitoredObjectDataSeries->dataProviderId);

          QUrl monitoredObjectUrl(monitoredObjectProvider->uri.c_str());

          if(monitoredObjectUrl.host() != url.host()
             || monitoredObjectUrl.port() != url.port()
             || monitoredObjectUrl.path().section("/", 1, 1) != url.path().section("/", 1, 1))
          {
            logger->log(ViewLogger::ERROR_MESSAGE, "Data to join is in a different DB.", logId);
            TERRAMA2_LOG_ERROR() << QObject::tr("Cannot join data from a different DB source!");
            continue;
          }

          if(monitoredObjectDataSeries->datasetList.empty())
          {
            logger->log(ViewLogger::ERROR_MESSAGE, "No join data.", logId);
            TERRAMA2_LOG_ERROR() << QObject::tr("Cannot join data from a different DB source!");
            continue;
          }

          const terrama2::core::DataSetPtr monitoredObjectDataset = monitoredObjectDataSeries->datasetList.at(0);

          std::string monitoredObjectTableName = dataAccessorPostGis->getDataSetTableName(monitoredObjectDataset);

          std::unique_ptr< te::da::DataSetType > dataSetType = datasource->getDataSetType(monitoredObjectTableName);

          std::string pk = dataSetType->getPrimaryKey()->getProperties().at(0)->getName();

          auto& propertiesVector = dataSetType->getProperties();

          joinSQL = "SELECT ";

          for(auto& property : propertiesVector)
          {
            const std::string& propertyName = property->getName();
            joinSQL += "t1." + propertyName + " as monitored_" + propertyName + ", ";
          }

          joinSQL += "t2.* ";
          joinSQL += "FROM " + monitoredObjectTableName + " as t1 , " + tableName + " as t2 ";
          joinSQL += "WHERE t1." + pk + " = t2." + pk;

          if(!modelDataSetType)
          {
            modelDataSetType.reset(dataSetType.release());
          }
        }

        registerPostgisTable(inputDataProvider->name,
                             connInfo,
                             layerName,
                             viewPtr->viewName,
                             modelDataSetType,
                             timestampPropertyName,
                             joinSQL);

        QJsonObject layer;
        layer.insert("layer", QString::fromStdString(layerName));
        layersArray.push_back(layer);
      }
    }

    // Register style
    auto legend = viewPtr->legendPerDataSeries.find(inputDataSeries->id);

    if(legend != viewPtr->legendPerDataSeries.end())
    {
      std::string legendName = viewPtr->viewName + "style" + std::to_string(inputDataSeries->id);
      registerStyle(legendName, legend->second, modelDataSetType);
    }

    // TODO: assuming that only has one dataseries, overwriting answer
    jsonAnswer.insert("workspace", QString::fromStdString(workspace()));
    jsonAnswer.insert("style", QString::fromStdString(styleName));
    jsonAnswer.insert("layers_list", layersArray);
  }

  return jsonAnswer;
}

terrama2::services::view::core::MapsServerPtr terrama2::services::view::core::GeoServer::make(te::core::URI uri)
{
  return std::make_shared<GeoServer>(uri);
}

std::string terrama2::services::view::core::GeoServer::getGeomTypeString(const te::gm::GeomType& geomType) const
{
  switch(geomType)
  {
    case te::gm::GeomType::MultiPolygonType :
    case te::gm::GeomType::MultiPolygonZType :
    case te::gm::GeomType::MultiPolygonMType :
    case te::gm::GeomType::MultiPolygonZMType :
      return "MultiPolygon";
    case te::gm::GeomType::GeometryType :
    case te::gm::GeomType::GeometryZType :
    case te::gm::GeomType::GeometryMType :
    case te::gm::GeomType::GeometryZMType :
      return "Geometry";
    case te::gm::GeomType::PointType :
    case te::gm::GeomType::PointZType :
    case te::gm::GeomType::PointMType :
    case te::gm::GeomType::PointZMType :
    case te::gm::GeomType::PointKdType :
      return "Point";
    case te::gm::GeomType::MultiPointType :
    case te::gm::GeomType::MultiPointZType :
    case te::gm::GeomType::MultiPointMType :
    case te::gm::GeomType::MultiPointZMType :
      return "MultiPoint";
    case te::gm::GeomType::LineStringType :
    case te::gm::GeomType::LineStringZType :
    case te::gm::GeomType::LineStringMType :
    case te::gm::GeomType::LineStringZMType :
      return "LineString";
    case te::gm::GeomType::MultiLineStringType :
    case te::gm::GeomType::MultiLineStringZType :
    case te::gm::GeomType::MultiLineStringMType :
    case te::gm::GeomType::MultiLineStringZMType :
      return "MultiLineString";
    case te::gm::GeomType::PolygonType :
    case te::gm::GeomType::PolygonZType :
    case te::gm::GeomType::PolygonMType :
    case te::gm::GeomType::PolygonZMType :
      return "Polygon";
    default:
      QString errMsg = QObject::tr("Error at register PostGis Table, unknow geometry type. ");
      TERRAMA2_LOG_ERROR() << errMsg;
      throw ViewGeoserverException() << ErrorDescription(errMsg);
      break;
  }
}

