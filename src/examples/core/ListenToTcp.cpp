
// TerraMA2

#include <terrama2/core/network/TcpManager.hpp>
#include <terrama2/core/data-model/DataManager.hpp>
#include <terrama2/core/data-model/DataProvider.hpp>
#include <terrama2/core/data-model/DataSeries.hpp>
#include <terrama2/core/data-model/DataSet.hpp>
#include <terrama2/core/data-model/DataSetOccurrence.hpp>
#include <terrama2/core/network/TcpSignals.hpp>
#include <terrama2/core/utility/JSonUtils.hpp>
#include <terrama2_config.hpp>

//STL
#include <iostream>

// Qt
#include <QUrl>
#include <QTimer>
#include <QObject>
#include <QCoreApplication>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTcpSocket>

class MockDataManager : public terrama2::core::DataManager
{
  public:
    MockDataManager() {}

    virtual ~MockDataManager() = default;
    MockDataManager(const MockDataManager& other) = default;
    MockDataManager(MockDataManager&& other) = default;
    MockDataManager& operator=(const MockDataManager& other) = default;
    MockDataManager& operator=(MockDataManager&& other) = default;

    virtual void addFromJSON(const QJsonValue& jsonValue) override
    {
      QJsonDocument doc(jsonValue.toObject());
      std::cout << QString(doc.toJson()).toStdString() << std::endl;
    }

};

terrama2::core::DataProviderPtr buildInputProvider()
{
  terrama2::core::DataProvider* dataProvider = new terrama2::core::DataProvider();
  terrama2::core::DataProviderPtr dataProviderPtr(dataProvider);

  QString uri = QString("file://%1/PCD_serrmar_INPE").arg(TERRAMA2_DATA_DIR);
  dataProvider->id = 1;
  dataProvider->projectId = 1;
  dataProvider->name = "Provider";
  dataProvider->description = "Testing provider";
  dataProvider->intent = terrama2::core::DataProvider::COLLECTOR_INTENT;
  dataProvider->uri = uri.toStdString();
  dataProvider->active = true;

  return dataProviderPtr;
}

terrama2::core::DataSeriesPtr buildInputDataSeries()
{
  // DataSeries information
  terrama2::core::DataSeries* dataSeries = new terrama2::core::DataSeries();
  terrama2::core::DataSeriesPtr dataSeriesPtr(dataSeries);
  dataSeries->id = 1;
  dataSeries->name = "DataProvider queimadas local";
  dataSeries->semantics.name = "OCCURRENCE-mvf";
  dataSeries->dataProviderId = 1;

  terrama2::core::DataSetOccurrence* dataSet = new terrama2::core::DataSetOccurrence();
  dataSeries->datasetList.emplace_back(dataSet);
  dataSet->id = 1;
  dataSet->active = true;
  dataSet->format.emplace("mask", "fires.csv");
  dataSet->format.emplace("timezone", "+00");
  dataSet->format.emplace("srid", "4326");

  return dataSeriesPtr;
}

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  QJsonArray array;
  array.push_back(terrama2::core::toJson(buildInputProvider()));
  array.push_back(terrama2::core::toJson(buildInputDataSeries()));

  QJsonDocument doc(array);

  terrama2::core::TcpManager tcpManager;
  std::shared_ptr<terrama2::core::DataManager> dataManager = std::make_shared<MockDataManager>();
  tcpManager.listen(dataManager, QHostAddress::Any, 30000);


  QByteArray bytearray;
  QDataStream out(&bytearray, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_2);

  out << static_cast<uint32_t>(0);
  out << terrama2::core::TcpSignals::DATA_SIGNAL;
  out << doc.toJson();
  out.device()->seek(0);
  out << static_cast<uint32_t>(bytearray.size() - sizeof(uint32_t));

  QTcpSocket socket;
  socket.connectToHost("localhost", 30000);
  socket.write(bytearray);

  QTimer timer;
  QObject::connect(&timer, SIGNAL(timeout()), QCoreApplication::instance(), SLOT(quit()));
  timer.start(30000);
  app.exec();

  return 0;
}
