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
  \file terrama2/core/utility/Raii.hpp
  \brief Generic classes for RAII control.
  \author Jano Simas
*/


#ifndef __TERRAMA2_CORE_UTILITY_RAII_HPP__
#define __TERRAMA2_CORE_UTILITY_RAII_HPP__

//TerraMA2
#include "Logger.hpp"

//STd
#include <exception>

// Libcurl
#include <curl/curl.h>


namespace terrama2
{
  namespace core
  {
    //! Class for Resource Acquisition Is Initialization (RAII) of classes that need to open and close.
    template<class T> class OpenClose
    {
    public:
      /*!
        \brief The constructor will open the object and keep a reference to it.
        \note The object must not be open, no verification is made.
        \param obj Object to be open.
       */
      OpenClose(T& obj)
        : obj_(obj)
      {
        obj_->open();
      }

      /*!
        \brief The destructor will close the object.
        Any esception thrown will be caught and logged, if unable to log nothing will be done.
       */
      ~OpenClose()
      {
        try
        {
          obj_->close();
        }
        catch(boost::exception&)
        {
          try
          {
            TERRAMA2_LOG_ERROR() << "boost::exception: Could not close object";
          }
          catch(...) { }
        }
        catch(std::exception&)
        {
          try
          {
            TERRAMA2_LOG_ERROR() << "std::exception: Could not close object";
          }
          catch(...) { }
        }
        catch(...) { }
      }

    private:
      T& obj_;

    };

    //! Class for Resource Acquisition Is Initialization (RAII) of FILE*.
    class FilePtr
    {
    public:

      //! Constructs a null pointer to a file
      FilePtr()
        : file_(nullptr)
      {
      }

      //! Constructs and open the file fileName with flags
      FilePtr(const std::string& fileName, const std::string& flags)
        : file_(nullptr)
      {
        open(fileName, flags);
      }

      //! Open the file fileName with flags
      void open(const std::string& fileName, const std::string& flags)
      {
        close();
        file_ = std::fopen(fileName.c_str(), flags.c_str());
      }

      //! Closes the file if open.
      void close()
      {
        if(file_)
        {
          std::fclose(file_);
          file_ = nullptr;
        }
      }

      //! Closes the file if open.
      ~FilePtr()
      {
        close();
      }

      //! Conversion to FILE* operator
      operator std::FILE*() const
      {
        return file_;
      }


      std::FILE* file() const
      {
        return file_;
      }

    private:
      std::FILE* file_;
    };

    //! Class for Resource Acquisition Is Initialization (RAII) of Curl.
    class CurlOpener
    {
      public:

       //! Constructor.
       CurlOpener()
       {
         curl_ = curl_easy_init();
       } 

       /*!
           The init function performs the function curl_easy_cleanup closing all handle connections 
           curl and then performs the initialization of the curl.
       */
       void init()
       {
         curl_easy_cleanup(curl_);
         curl_ = curl_easy_init();
       }

       //! Assume ownership of curl.
       CURL* fcurl() const
       {
         return curl_;
       }

       /*! When CurlOpener destructor is called, the function curl_easy_cleanup is used automatically. 
           The function curl_easy_cleanup close all connections this handle curl.
       */
       ~CurlOpener()
       {
         curl_easy_cleanup(curl_);
       }

      private:
        CURL* curl_; //!< Attribute for Handler Curl.
     };

  }
}

#endif //__TERRAMA2_CORE_UTILITY_RAII_HPP__
