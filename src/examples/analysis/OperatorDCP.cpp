
#include <terrama2/core/Shared.hpp>
#include <terrama2/core/utility/Utils.hpp>
#include <terrama2/core/utility/ServiceManager.hpp>
#include <terrama2/core/data-model/DataProvider.hpp>
#include <terrama2/core/data-model/DataSeries.hpp>
#include <terrama2/core/data-model/DataSet.hpp>
#include <terrama2/core/data-model/DataSetDcp.hpp>

#include <terrama2/services/analysis/core/PythonInterpreter.hpp>
#include <terrama2/services/analysis/core/Analysis.hpp>
#include <terrama2/services/analysis/core/Service.hpp>
#include <terrama2/services/analysis/core/DataManager.hpp>

#include <terrama2/impl/Utils.hpp>
#include <terrama2/Config.hpp>

#include <iostream>

// QT
#include <QTimer>
#include <QCoreApplication>
#include <QUrl>


using namespace terrama2::services::analysis::core;

int main(int argc, char* argv[])
{
  terrama2::core::initializeTerraMA();

  terrama2::core::registerFactories();


  auto& serviceManager = terrama2::core::ServiceManager::getInstance();
  std::map<std::string, std::string> connInfo { {"PG_HOST", TERRAMA2_DATABASE_HOST},
                                                {"PG_PORT", TERRAMA2_DATABASE_PORT},
                                                {"PG_USER", TERRAMA2_DATABASE_USERNAME},
                                                {"PG_PASSWORD", TERRAMA2_DATABASE_PASSWORD},
                                                {"PG_DB_NAME", TERRAMA2_DATABASE_DBNAME},
                                                {"PG_CONNECT_TIMEOUT", "4"},
                                                {"PG_CLIENT_ENCODING", "UTF-8"}
                                              };
  serviceManager.setLogConnectionInfo(connInfo);

  terrama2::services::analysis::core::initInterpreter();

  QCoreApplication app(argc, argv);


  DataManagerPtr dataManager(new DataManager());

  QUrl uri;
  uri.setScheme("postgis");
  uri.setHost(TERRAMA2_DATABASE_HOST);
  uri.setPort(atoi(TERRAMA2_DATABASE_PORT));
  uri.setUserName(TERRAMA2_DATABASE_USERNAME);
  uri.setPassword(TERRAMA2_DATABASE_PASSWORD);
  uri.setPath("/");
  uri.setPath(uri.path() + TERRAMA2_DATABASE_DBNAME);

  // DataProvider information
  terrama2::core::DataProvider* outputDataProvider = new terrama2::core::DataProvider();
  terrama2::core::DataProviderPtr outputDataProviderPtr(outputDataProvider);
  outputDataProvider->id = 3;
  outputDataProvider->name = "DataProvider postgis";
  outputDataProvider->uri = uri.url().toStdString();
  outputDataProvider->intent = terrama2::core::DataProvider::PROCESS_INTENT;
  outputDataProvider->dataProviderType = "POSTGIS";
  outputDataProvider->active = true;

  dataManager->add(outputDataProviderPtr);

  // DataSeries information
  terrama2::core::DataSeries* outputDataSeries = new terrama2::core::DataSeries();
  terrama2::core::DataSeriesPtr outputDataSeriesPtr(outputDataSeries);
  outputDataSeries->id = 3;
  outputDataSeries->name = "Analysis result";
  outputDataSeries->semantics.code = "ANALYSIS_MONITORED_OBJECT-postgis";
  outputDataSeries->dataProviderId = outputDataProviderPtr->id;


  // DataSet information
  terrama2::core::DataSet* outputDataSet = new terrama2::core::DataSet();
  outputDataSet->active = true;
  outputDataSet->id = 2;
  outputDataSet->format.emplace("table_name", "analysis_result");

  outputDataSeries->datasetList.emplace_back(outputDataSet);


  dataManager->add(outputDataSeriesPtr);

  std::string script = "x = dcp.min(\"Serra do Mar\", \"pluvio\", 2, Buffer.OBJECT_PLUS_EXTERN)\n"
                       "add_value(\"min\", x)\n"
                       "x = dcp.max(\"Serra do Mar\", \"pluvio\", 2, Buffer.OBJECT_PLUS_EXTERN)\n"
                       "add_value(\"max\", x)\n"
                       "x = dcp.mean(\"Serra do Mar\", \"pluvio\", 2, Buffer.OBJECT_PLUS_EXTERN)\n"
                       "add_value(\"mean\", x)\n"
                       "x = dcp.median(\"Serra do Mar\", \"pluvio\", 2, Buffer.OBJECT_PLUS_EXTERN)\n"
                       "add_value(\"median\", x)\n"
                       "x = dcp.standardDeviation(\"Serra do Mar\", \"pluvio\", 2, Buffer.OBJECT_PLUS_EXTERN)\n"
                       "add_value(\"standardDeviation\", x)\n";

  Analysis analysis;
  analysis.id = 1;
  analysis.name = "Min DCP";
  analysis.script = script;
  analysis.scriptLanguage = PYTHON;
  analysis.type = MONITORED_OBJECT_TYPE;
  analysis.active = false;
  analysis.outputDataSeriesId = 3;

  analysis.metadata["INFLUENCE_TYPE"] = "1";
  analysis.metadata["INFLUENCE_RADIUS"] = "1";
  analysis.metadata["INFLUENCE_UNIT"] = "km";

  terrama2::core::DataProvider* dataProvider = new terrama2::core::DataProvider();
  terrama2::core::DataProviderPtr dataProviderPtr(dataProvider);
  dataProvider->name = "Provider";
  dataProvider->uri+=TERRAMA2_DATA_DIR;
  dataProvider->uri+="/shapefile";
  dataProvider->intent = terrama2::core::DataProvider::COLLECTOR_INTENT;
  dataProvider->dataProviderType = "FILE";
  dataProvider->active = true;
  dataProvider->id = 1;


  dataManager->add(dataProviderPtr);

  terrama2::core::DataSeries* dataSeries = new terrama2::core::DataSeries();
  terrama2::core::DataSeriesPtr dataSeriesPtr(dataSeries);
  dataSeries->dataProviderId = dataProvider->id;
  dataSeries->semantics.code = "STATIC_DATA-ogr";
  dataSeries->semantics.dataSeriesType = terrama2::core::DataSeriesSemantics::STATIC;
  dataSeries->name = "Monitored Object";
  dataSeries->id = 1;
  dataSeries->dataProviderId = 1;

  //DataSet information
  terrama2::core::DataSet* dataSet = new terrama2::core::DataSet;
  terrama2::core::DataSetPtr dataSetPtr(dataSet);
  dataSet->active = true;
  dataSet->format.emplace("mask", "estados_2010.shp");
  dataSet->format.emplace("srid", "4618");
  dataSet->format.emplace("identifier", "NOME");
  dataSet->id = 1;

  dataSeries->datasetList.push_back(dataSetPtr);
  dataManager->add(dataSeriesPtr);


  terrama2::core::DataProvider* dataProvider2 = new terrama2::core::DataProvider();
  terrama2::core::DataProviderPtr dataProvider2Ptr(dataProvider2);
  dataProvider2->name = "Provider";
  dataProvider2->uri+=TERRAMA2_DATA_DIR;
  dataProvider2->uri+="/PCD_serrmar_INPE";
  dataProvider2->intent = terrama2::core::DataProvider::COLLECTOR_INTENT;
  dataProvider2->dataProviderType = "FILE";
  dataProvider2->active = true;
  dataProvider2->id = 2;


  dataManager->add(dataProvider2Ptr);

  AnalysisDataSeries monitoredObjectADS;
  monitoredObjectADS.id = 1;
  monitoredObjectADS.dataSeriesId = dataSeriesPtr->id;
  monitoredObjectADS.type = DATASERIES_MONITORED_OBJECT_TYPE;


  //DataSeries information
  terrama2::core::DataSeries* dcpSeries = new terrama2::core::DataSeries;
  terrama2::core::DataSeriesPtr dcpSeriesPtr(dcpSeries);
  dcpSeries->dataProviderId = dataProvider2->id;
  dcpSeries->semantics.code = "DCP-inpe";
  dcpSeries->semantics.dataSeriesType = terrama2::core::DataSeriesSemantics::DCP;
  dcpSeries->name = "Serra do Mar";
  dcpSeries->id = 2;
  dcpSeries->dataProviderId = 2;

  //DataSet information
  terrama2::core::DataSetDcp* dcpDataset69034 = new terrama2::core::DataSetDcp;
  terrama2::core::DataSetDcpPtr dcpDataset69034Ptr(dcpDataset69034);
  dcpDataset69034->active = true;
  dcpDataset69034->format.emplace("mask", "69033.txt");
  dcpDataset69034->format.emplace("timezone", "-02:00");
  dcpDataset69034->dataSeriesId = 2;
  dcpDataset69034->id = 2;
  dcpDataset69034->position = std::shared_ptr<te::gm::Point>(new te::gm::Point(-44.46540, -23.00506, 4618, nullptr));
  dcpSeries->datasetList.push_back(dcpDataset69034Ptr);


  terrama2::core::DataSetDcp* dcpDataset30886 = new terrama2::core::DataSetDcp;
  terrama2::core::DataSetDcpPtr dcpDataset30886Ptr(dcpDataset30886);
  dcpDataset30886->active = true;
  dcpDataset30886->format.emplace("mask", "30886.txt");
  dcpDataset30886->format.emplace("timezone", "-02:00");
  dcpDataset30886->dataSeriesId = 2;
  dcpDataset30886->id = 3;
  dcpDataset30886->position = std::shared_ptr<te::gm::Point>(new te::gm::Point(-46.121, -23.758, 4618, nullptr));
  dcpSeries->datasetList.push_back(dcpDataset30886Ptr);

  AnalysisDataSeries dcpADS;
  dcpADS.id = 2;
  dcpADS.dataSeriesId = dcpSeriesPtr->id;
  dcpADS.type = ADDITIONAL_DATA_TYPE;
  dcpADS.metadata["INFLUENCE_TYPE"] = "RADIUS_CENTER";
  dcpADS.metadata["RADIUS"] = "50";

  dataManager->add(dcpSeriesPtr);

  std::vector<AnalysisDataSeries> analysisDataSeriesList;
  analysisDataSeriesList.push_back(dcpADS);
  analysisDataSeriesList.push_back(monitoredObjectADS);
  analysis.analysisDataSeriesList = analysisDataSeriesList;

  dataManager->add(analysis);

  // Starts the service and adds the analysis
  Service service(dataManager);
  service.start();
  service.addAnalysis(1);

  /*QTimer timer;
  QObject::connect(&timer, SIGNAL(timeout()), QCoreApplication::instance(), SLOT(quit()));
  timer.start(30000);*/

  app.exec();


  terrama2::core::finalizeTerralib();

  return 0;
}
