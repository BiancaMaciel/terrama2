
#include "../core/shared.hpp"
#include "../core/utility/Utils.hpp"
#include "../core/data-model/DataProvider.hpp"
#include "../core/data-model/DataSeries.hpp"
#include "../core/data-model/DataSetDcp.hpp"
#include "../impl/DataAccessorDcpInpe.hpp"

#include <iostream>


int main(int argc, char* argv[])
{
  terrama2::core::initializeTerralib();

  terrama2::core::DataProvider dataProvider;
  dataProvider.uri = "file:///home/jsimas/MyDevel/dpi/terrama2-extra/data/PCD_serrmar_INPE";
  dataProvider.intent = terrama2::core::DataProvider::COLLECTOR_INTENT;
  dataProvider.dataProviderType = 0;
  dataProvider.active = true;

  terrama2::core::DataSeries dataSeries;
  dataSeries.semantic = 0;

  dataSeries.datasetList.emplace_back(new terrama2::core::DataSetDcp());
  std::shared_ptr<terrama2::core::DataSetDcp> dataSet = std::dynamic_pointer_cast<terrama2::core::DataSetDcp>(dataSeries.datasetList.at(0));
  dataSet->active = true;
  dataSet->dateTimeColumnName = "N/A";
  dataSet->format.emplace("mask", "30885.txt");

  dataProvider.dataSeriesList.push_back(dataSeries);

  // angra.txt
  terrama2::core::Filter filter;

  terrama2::core::DataAccessorDcpInpe accessor(dataProvider, dataSeries);
  terrama2::core::DcpSeriesPtr dcpSeries = accessor.getDcpSeries(filter);

  assert(dcpSeries->dcpList().size() == 1);

  std::shared_ptr<te::mem::DataSet> teDataSet = dcpSeries->dcpList().at(0).second;

  int dateColumn = -1;
  std::string names, types;
  for(int i = 0; i < teDataSet->getNumProperties(); ++i)
  {
    std::string name = teDataSet->getPropertyName(i);
    names+= name + "\t";
    if(name == "DateTime")
    {
      types+= "DataTime\t";
      dateColumn = i;
    }
    else
      types+= "Double\t";
  }

  std::cout << names << std::endl;
  std::cout << types << std::endl;

  teDataSet->moveBeforeFirst();
  while(teDataSet->moveNext())
  {
    for(int i = 0; i < teDataSet->getNumProperties(); ++i)
    {
      if(teDataSet->isNull(i))
      {
        std::cout << "NULL";
        continue;
      }

      if(i == dateColumn)
      {
        std::shared_ptr<te::dt::DateTime> dateTime =  teDataSet->getDateTime(i);
        std::cout << dateTime->toString();
      }
      else
      {
        double value =  teDataSet->getDouble(i);
        std::cout << value;
      }

      std::cout << "\t";
    }
    std::cout << std::endl;
  }


  terrama2::core::DataProvider dataProvider2;
  terrama2::core::DataSeries dataSeries2;
  // terrama2::core::DcpStoragerPtr storager = Factory::getDcpStorager(dataProvider2, dataSeries2);
  // storager->store(dcpSeries);

  terrama2::core::finalizeTerralib();

  return 0;
}
