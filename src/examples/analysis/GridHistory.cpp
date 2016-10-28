#include <terrama2/core/Shared.hpp>
#include <terrama2/core/utility/Utils.hpp>
#include <terrama2/core/utility/TerraMA2Init.hpp>
#include <terrama2/core/utility/ServiceManager.hpp>
#include <terrama2/core/utility/SemanticsManager.hpp>
#include <terrama2/core/data-model/DataProvider.hpp>
#include <terrama2/core/data-model/DataSeries.hpp>
#include <terrama2/core/data-model/DataSet.hpp>
#include <terrama2/core/data-model/DataSetGrid.hpp>

#include <terrama2/services/analysis/core/python/PythonInterpreter.hpp>
#include <terrama2/services/analysis/core/utility/PythonInterpreterInit.hpp>
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
  terrama2::core::TerraMA2Init terramaRaii("example", 0);

  terrama2::core::registerFactories();


  auto& serviceManager = terrama2::core::ServiceManager::getInstance();
  std::map<std::string, std::string> connInfo { {"PG_HOST",            TERRAMA2_DATABASE_HOST},
    {"PG_PORT",            TERRAMA2_DATABASE_PORT},
    {"PG_USER",            TERRAMA2_DATABASE_USERNAME},
    {"PG_PASSWORD",        TERRAMA2_DATABASE_PASSWORD},
    {"PG_DB_NAME",         TERRAMA2_DATABASE_DBNAME},
    {"PG_CONNECT_TIMEOUT", "4"},
    {"PG_CLIENT_ENCODING", "UTF-8"}
  };
  serviceManager.setLogConnectionInfo(connInfo);

  terrama2::services::analysis::core::PythonInterpreterInit pythonInterpreterInit;

  {
    QCoreApplication app(argc, argv);


    DataManagerPtr dataManager(new DataManager());

    QUrl uri;
    uri.setScheme("postgis");
    uri.setHost(QString::fromStdString(TERRAMA2_DATABASE_HOST));
    uri.setPort(std::stoi(TERRAMA2_DATABASE_PORT));
    uri.setUserName(QString::fromStdString(TERRAMA2_DATABASE_USERNAME));
    uri.setPassword(QString::fromStdString(TERRAMA2_DATABASE_PASSWORD));
    uri.setPath(QString::fromStdString("/" + TERRAMA2_DATABASE_DBNAME));

    // DataProvider information
    terrama2::core::DataProvider* dataProvider = new terrama2::core::DataProvider();
    terrama2::core::DataProviderPtr dataProviderPtr(dataProvider);
    dataProvider->uri = "file://";
    dataProvider->uri += TERRAMA2_DATA_DIR;
    dataProvider->uri += "/geotiff/historical";

    dataProvider->intent = terrama2::core::DataProviderIntent::COLLECTOR_INTENT;
    dataProvider->dataProviderType = "FILE";
    dataProvider->active = true;
    dataProvider->id = 1;
    dataProvider->name = "Local Geotiff";

    dataManager->add(dataProviderPtr);

    auto& semanticsManager = terrama2::core::SemanticsManager::getInstance();

    //DataSeries information
    terrama2::core::DataSeries* outputDataSeries = new terrama2::core::DataSeries();
    terrama2::core::DataSeriesPtr outputDataSeriesPtr(outputDataSeries);
    outputDataSeries->semantics = semanticsManager.getSemantics("GRID-geotiff");
    outputDataSeries->name = "Output Grid";
    outputDataSeries->id = 5;
    outputDataSeries->dataProviderId = 1;

    terrama2::core::DataSetGrid* outputDataSet = new terrama2::core::DataSetGrid();
    outputDataSet->active = true;
    outputDataSet->format.emplace("mask", "output_history_grid.tif");

    outputDataSeries->datasetList.emplace_back(outputDataSet);

    dataManager->add(outputDataSeriesPtr);

    dataManager->add(dataProviderPtr);



    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Data Series 1
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    terrama2::core::DataSeries* dataSeries1 = new terrama2::core::DataSeries();
    terrama2::core::DataSeriesPtr dataSeries1Ptr(dataSeries1);
    dataSeries1->semantics = semanticsManager.getSemantics("GRID-geotiff");
    dataSeries1->name = "geotiff 1";
    dataSeries1->id = 1;
    dataSeries1->dataProviderId = 1;

    terrama2::core::DataSetGrid* dataSet1 = new terrama2::core::DataSetGrid();
    dataSet1->active = true;
    dataSet1->format.emplace("mask", "%YYYY%MM%DD_%hh%mm%ss.tif");
    dataSet1->id = 1;

    dataSeries1->datasetList.emplace_back(dataSet1);

    AnalysisDataSeries gridADS1;
    gridADS1.id = 1;
    gridADS1.dataSeriesId = dataSeries1Ptr->id;
    gridADS1.type = AnalysisDataSeriesType::ADDITIONAL_DATA_TYPE;


    dataManager->add(dataSeries1Ptr);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Analysis
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    Analysis* analysis = new Analysis;
    AnalysisPtr analysisPtr(analysis);

    analysis->id = 1;
    analysis->name = "Grid Sample";
    analysis->script = "return grid.history.interval.min(\"geotiff 1\", \"30d\", \"0d\")";
    analysis->scriptLanguage = ScriptLanguage::PYTHON;
    analysis->type = AnalysisType::GRID_TYPE;
    analysis->active = true;
    analysis->outputDataSeriesId = 5;
    analysis->serviceInstanceId = 1;

    std::vector<AnalysisDataSeries> analysisDataSeriesList;
    analysisDataSeriesList.push_back(gridADS1);
    analysis->analysisDataSeriesList = analysisDataSeriesList;

    analysis->schedule.frequency = 1;
    analysis->schedule.frequencyUnit = "min";

    AnalysisOutputGrid* outputGrid = new AnalysisOutputGrid();
    AnalysisOutputGridPtr outputGridPtr(outputGrid);

    outputGrid->analysisId = 1;
    outputGrid->interpolationMethod = InterpolationMethod::BILINEAR;
    outputGrid->interestAreaType = InterestAreaType::SAME_FROM_DATASERIES;
    outputGrid->interestAreaDataSeriesId = 1;
    outputGrid->resolutionType = ResolutionType::SAME_FROM_DATASERIES;
    outputGrid->resolutionDataSeriesId = 1;
    outputGrid->interpolationDummy = 266;

    analysis->outputGridPtr = outputGridPtr;

    dataManager->add(analysisPtr);

    // Starts the service and adds the analysis
    Service service(dataManager);
    terrama2::core::ServiceManager::getInstance().setInstanceId(1);

    auto logger = std::make_shared<AnalysisLogger>();
    logger->setConnectionInfo(connInfo);
    service.setLogger(logger);

    service.start();
    service.addProcessToSchedule(analysisPtr);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &service, &Service::stopService);
    QObject::connect(&timer, SIGNAL(timeout()), QCoreApplication::instance(), SLOT(quit()));
    timer.start(10000);

    app.exec();

  }

  return 0;
}
