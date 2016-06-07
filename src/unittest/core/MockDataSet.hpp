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
  \file unittest/core/MockDataSet.hpp

  \brief Mock for TerraLib Data Set

  \author Vinicius Campanha
*/

#ifndef __TERRAMA2_UNITTEST_CORE_MOCKDATASET_HPP__
#define __TERRAMA2_UNITTEST_CORE_MOCKDATASET_HPP__

// TerraLib
#include <terralib/dataaccess/dataset/DataSet.h>

// GMock
#include <gmock/gmock.h>


namespace te
{
  namespace da
  {

    class MockDataSet : public DataSet
    {
    public:

      MockDataSet() = default;

      virtual ~MockDataSet() = default;

      MOCK_METHOD0(EnvelopePtrReturn, te::gm::Envelope*());

      virtual std::auto_ptr<te::gm::Envelope> getExtent(std::size_t i) override
      {
        return std::auto_ptr<te::gm::Envelope>(EnvelopePtrReturn());
      }

      MOCK_CONST_METHOD0(ByteArrayPtrReturn, te::dt::ByteArray*());


      virtual std::auto_ptr<te::dt::ByteArray> getByteArray(std::size_t i) const override
      {
        return std::auto_ptr<te::dt::ByteArray>(ByteArrayPtrReturn());
      }

      virtual std::auto_ptr<te::dt::ByteArray> getByteArray(const std::string& name) const override
      {
        return std::auto_ptr<te::dt::ByteArray>(ByteArrayPtrReturn());
      }

      MOCK_CONST_METHOD0(GeometryPtrReturn, te::gm::Geometry*());

      virtual std::auto_ptr<te::gm::Geometry> getGeometry(std::size_t i) const override
      {
        return std::auto_ptr<te::gm::Geometry>(GeometryPtrReturn());
      }

      virtual std::auto_ptr<te::gm::Geometry> getGeometry(const std::string& name) const override
      {
        return std::auto_ptr<te::gm::Geometry>(GeometryPtrReturn());
      }

      MOCK_CONST_METHOD0(RasterPtrReturn, te::rst::Raster*());

      virtual std::auto_ptr<te::rst::Raster> getRaster(std::size_t i) const override
      {
        return std::auto_ptr<te::rst::Raster>(RasterPtrReturn());
      }

      virtual std::auto_ptr<te::rst::Raster> getRaster(const std::string& name) const override
      {
        return std::auto_ptr<te::rst::Raster>(RasterPtrReturn());
      }

      MOCK_CONST_METHOD0(DateTimePtrReturn, te::dt::DateTime*());

      virtual std::auto_ptr<te::dt::DateTime> getDateTime(std::size_t i) const override
      {
        return std::auto_ptr<te::dt::DateTime>(DateTimePtrReturn());
      }

      virtual std::auto_ptr<te::dt::DateTime> getDateTime(const std::string& name) const override
      {
        return std::auto_ptr<te::dt::DateTime>(DateTimePtrReturn());
      }

      MOCK_CONST_METHOD0(ArrayPtrReturn, te::dt::Array*());

      virtual std::auto_ptr<te::dt::Array> getArray(std::size_t i)  const override
      {
        return std::auto_ptr<te::dt::Array>(ArrayPtrReturn());
      }

      virtual std::auto_ptr<te::dt::Array> getArray(const std::string& name) const override
      {
        return std::auto_ptr<te::dt::Array>(ArrayPtrReturn());
      }

      MOCK_CONST_METHOD0(AbstractDataPtrReturn, te::dt::AbstractData*());

      virtual std::auto_ptr<te::dt::AbstractData> getValue(std::size_t i) const override
      {
        return std::auto_ptr<te::dt::AbstractData>(AbstractDataPtrReturn());
      }

      virtual std::auto_ptr<te::dt::AbstractData> getValue(const std::string& name) const override
      {
        return std::auto_ptr<te::dt::AbstractData>(AbstractDataPtrReturn());
      }


      MOCK_CONST_METHOD0(getTraverseType,
                         te::common::TraverseType());
      MOCK_CONST_METHOD0(getAccessPolicy,
                         te::common::AccessPolicy());
      MOCK_CONST_METHOD0(getNumProperties,
                         std::size_t());
      MOCK_CONST_METHOD1(getPropertyDataType,
                         int(std::size_t i));
      MOCK_CONST_METHOD1(getPropertyName,
                         std::string(std::size_t i));
      MOCK_CONST_METHOD1(getPropertyCharEncoding,
                         te::core::EncodingType(std::size_t i));
      MOCK_CONST_METHOD1(getDatasetNameOfProperty,
                         std::string(std::size_t i));
      MOCK_CONST_METHOD0(isEmpty,
                         bool());
      MOCK_CONST_METHOD0(isConnected,
                         bool());
      MOCK_CONST_METHOD0(size,
                         std::size_t());

      MOCK_METHOD0(moveNext,
                   bool());
      MOCK_METHOD0(movePrevious,
                   bool());
      MOCK_METHOD0(moveBeforeFirst,
                   bool());
      MOCK_METHOD0(moveFirst,
                   bool());
      MOCK_METHOD0(moveLast,
                   bool());
      MOCK_METHOD1(move,
                   bool(std::size_t i));
      MOCK_CONST_METHOD0(isAtBegin,
                         bool());
      MOCK_CONST_METHOD0(isBeforeBegin,
                         bool());
      MOCK_CONST_METHOD0(isAtEnd,
                         bool());
      MOCK_CONST_METHOD0(isAfterEnd,
                         bool());
      MOCK_CONST_METHOD1(getChar,
                         char(std::size_t i));
      MOCK_CONST_METHOD1(getChar,
                         char(const std::string& name));
      MOCK_CONST_METHOD1(getUChar,
                         unsigned char(std::size_t i));
      MOCK_CONST_METHOD1(getUChar,
                         unsigned char(const std::string& name));
      MOCK_CONST_METHOD1(getInt16,
                         boost::int16_t(std::size_t i));
      MOCK_CONST_METHOD1(getInt16,
                         boost::int16_t(const std::string& name));
      MOCK_CONST_METHOD1(getInt32,
                         boost::int32_t(std::size_t i));
      MOCK_CONST_METHOD1(getInt32,
                         boost::int32_t(const std::string& name));
      MOCK_CONST_METHOD1(getInt64,
                         boost::int64_t(std::size_t i));
      MOCK_CONST_METHOD1(getInt64,
                         boost::int64_t(const std::string& name));
      MOCK_CONST_METHOD1(getBool,
                         bool(std::size_t i));
      MOCK_CONST_METHOD1(getBool,
                         bool(const std::string& name));
      MOCK_CONST_METHOD1(getFloat,
                         float(std::size_t i));
      MOCK_CONST_METHOD1(getFloat,
                         float(const std::string& name));
      MOCK_CONST_METHOD1(getDouble,
                         double(std::size_t i));
      MOCK_CONST_METHOD1(getDouble,
                         double(const std::string& name));
      MOCK_CONST_METHOD1(getNumeric,
                         std::string(std::size_t i));
      MOCK_CONST_METHOD1(getNumeric,
                         std::string(const std::string& name));
      MOCK_CONST_METHOD1(getString,
                         std::string(std::size_t i));
      MOCK_CONST_METHOD1(getString,
                         std::string(const std::string& name));

      MOCK_CONST_METHOD2(getAsString,
                         std::string(std::size_t, int));
      MOCK_CONST_METHOD2(getAsString,
                         std::string(std::string, int));
      MOCK_CONST_METHOD1(isNull,
                         bool(std::size_t i));
      MOCK_CONST_METHOD1(isNull,
                         bool(const std::string& name));
    };

  }  // namespace da
}  // namespace te

#endif // __TERRAMA2_UNITTEST_CORE_MOCKDATASET_HPP__
