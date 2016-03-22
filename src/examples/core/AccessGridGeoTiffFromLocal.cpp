
#include "../core/shared.hpp"
#include "../core/utility/Utils.hpp"
#include "../core/data-model/DataProvider.hpp"
#include "../core/data-model/DataSeries.hpp"
#include "../core/data-model/DataSetGrid.hpp"
#include "../impl/DataAccessorGeoTiff.hpp"
#include "../core/data-access/GridSeries.hpp"

#include <iostream>


int main(int argc, char* argv[])
{
  terrama2::core::initializeTerralib();

//DataProvider information
  terrama2::core::DataProvider dataProvider;
  dataProvider.uri = "file:///home/jsimas/MyDevel/dpi/terrama2-extra/test_data/geotiff";
  dataProvider.intent = terrama2::core::DataProvider::COLLECTOR_INTENT;
  dataProvider.dataProviderType = 0;
  dataProvider.active = true;

//DataSeries information
  terrama2::core::DataSeries dataSeries;
  dataSeries.semantics.name = "GRID-geotiff";

  dataSeries.datasetList.emplace_back(new terrama2::core::DataSetGrid());
  //DataSet information
  std::shared_ptr<terrama2::core::DataSetGrid> dataSet = std::dynamic_pointer_cast<terrama2::core::DataSetGrid>(dataSeries.datasetList.at(0));
  dataSet->active = true;
  dataSet->format.emplace("mask", "L5219076_07620040908_r3g2b1.tif");

  dataProvider.dataSeriesList.push_back(dataSeries);

  //empty filter
  terrama2::core::Filter filter;
//accessing data
  terrama2::core::DataAccessorGeoTiff accessor(dataProvider, dataSeries);
  terrama2::core::GridSeriesPtr gridSeries = accessor.getGridSeries(filter);

  assert(gridSeries->gridList().size() == 1);

  // std::shared_ptr<te::mem::DataSet> teDataSet = gridSeries->gridList().at(0).second;


  terrama2::core::finalizeTerralib();

  return 0;
}
