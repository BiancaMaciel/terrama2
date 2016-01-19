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
  \file terrama2/collector/Storager.hpp

  \brief Store a temporary terralib DataSet into the permanent storage area.

  \author Jano Simas
*/

#ifndef __TERRAMA2_COLLECTOR_STORAGER_HPP__
#define __TERRAMA2_COLLECTOR_STORAGER_HPP__

#include "../core/DataSetItem.hpp"
#include "TransferenceData.hpp"

//terralib
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>

//Boost
#include <boost/noncopyable.hpp>

namespace terrama2
{
  namespace collector
  {

    /*!
          \brief The Storager class store the data in the final storage area and format.

          The Storager is responsible for creating the final storaging area and
          converting the data (terralib) to the appropriate format.

         */
    class Storager : public boost::noncopyable
    {
    public:
      //! Constructor. Stores metadata for later use.
      Storager(const std::map<std::string, std::string>& metadata);

      /*!
          \brief Store a temporary data set in it's final storage area and format.
          \param Code name for the storage dataset, may not be used if defined in the storage metadata

          \return Uri of the final storage.

          \pre Terralib should be initialized.
         */
      virtual void store(std::vector<TransferenceData>& transferenceDataVec) = 0;


    protected:
      std::map<std::string, std::string> metadata_;//!< Specifications of where and how to store the data.

    };

    typedef std::shared_ptr<Storager> StoragerPtr; //!< Shared pointer to Storager
  }
}


#endif //__TERRAMA2_COLLECTOR_STORAGER_HPP__
