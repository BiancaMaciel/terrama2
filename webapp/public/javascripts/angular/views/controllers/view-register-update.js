define([], function() {
  'use strict';

  /**
   * It represents a Controller to handle View form registration.
   * @class ViewRegistration
   */
  function ViewRegisterUpdate($scope, i18n, ViewService, $log, $http, $timeout, MessageBoxService, $window, DataSeriesService, Service, StyleConstants, StringFormat, StyleType, ColorFactory, FieldRetriever) {
    /**
     * @type {ViewRegisterUpdate}
     */
    var self = this;
    /**
     * It retrieves a configuration from main window.
     * 
     * @type {Object}
     */
    var config = window.configuration;
    /**
     * It handles alert box display
     * 
     * @type {Object}
     */
    self.MessageBoxService = MessageBoxService;
    /**
     * It defines available DataSeries type from DataSeries Service
     * @enum string
     */
    self.DataSeriesType = DataSeriesService.DataSeriesType;
    /**
     * It handles Service Instance model
     * 
     * @type {Object}
     */
    self.ServiceInstance = Service;
    /** 
     * @type {Object}
     */
    self.scheduleOptions = {};
    /**
     * It handles if should show schedule or not. It may be changed on view data series change
     * 
     * @type {boolean}
     */
    self.isDynamic = false;
    /**
     * It defines a list of Service with type View. It will be filled out in ng-options generation
     * @type {Service[]}
     */
    self.filteredServices = [];
    /**
     * Flag to handle if is Updating or Registering
     * 
     * @type {Boolean}
     */
    self.isUpdating = config.view ? true : false;
    /**
     * It defines a list of cached data series
     * @type {Object[]}
     */
    self.dataSeries = [];
    /**
     * It configures color picker (Angular Color Picker dependency)
     * @type {Object}
     */
    self.colorOptions = {
      format: 'hex'
    };
    /**
     * It defines min color HEX representation. Used for View GRID Data Series
     * @type {string}
     */
    self.minColor = "";
    /**
     * It defines max color HEX representation. Used for View GRID Data Series
     * @type {string}
     */
    self.maxColor = "";
    /**
     * It defines min value of style. Used for View GRID Data Series
     * @type {number}
     */
    self.minValue = null;
    /**
     * It defines max value of style. Used for View GRID Data Series
     * @type {number}
     */
    self.maxValue = null;
    /**
     * It defines the available styles for GEOMETRIC_OBJECT
     * @type {StyleType}
     */
    self.StyleType = StyleType;
    /**
     * It defines a list of available columns to display
     * @type {string[]}
     */
    self.columnsList = [];
    /**
     * It defines a list of colors used to generate legend
     * @type {Object[]}
     */
    self.colors = [];
    /**
     * It tries to sets begin and end color based in table row selection
     */
    self.handleColor = function() {
      var initial = self.colors[0];
      var final = self.colors[self.colors.length - 1];
      self.view.beginColor = initial.color;
      self.view.endColor = final.color;
    };
    /**
     * It defines a event listeners for color handling
     */
    self.events = {
      onChange: function(api, color, $event) {
        handleColor();
      }
    };
    /**
     * It builds a style from data series semantics
     * 
     * @param {DataSeriesService.DataSeriesType} semanticsTypeName
     * @returns {string}
     */
    var makeStyle = function(semanticsTypeName) {
      var targetStyle = "";
      switch(semanticsTypeName) {
        case DataSeriesService.DataSeriesType.GEOMETRIC_OBJECT:
          targetStyle = StyleConstants.COMMON;
          self.schedule = {};
          break;
        case DataSeriesService.DataSeriesType.GRID:
          targetStyle = StyleConstants.GRID;
          break;
        default:
          targetStyle = StyleConstants.COMMON;
      }
      return targetStyle;
    };
    /**
     * It performs a colors generation based in band selected and color. It starts from white color ("#000000").
     */
    self.generateColors = function() {
      var colorsArr = ColorFactory.generateColor(self.view.beginColor, self.view.endColor, self.view.bands + 1).reverse();
      for(var i = 1; i < colorsArr.length; ++i) {
        colorsArr[i] = {title: i18n.__("Color") + " " + i, color: colorsArr[i], value: i, isDefault: false};
      }
      var firstColor = colorsArr[0];
      colorsArr[0] = {
        title: i18n.__("Default"),
        color: firstColor,
        isDefault: true,
        value: 0
      };

      self.colors = colorsArr;
      // self.colors = ColorFactory.generateColor("#000000", self.view.colorBand, self.view.bands);
    };

    /**
     * It retrieves all data provider type to get HTTP fields
     */
    $http.get("/api/DataProviderType", {}).then(function(response) {
      var data = response.data;
      // Setting HTTP default syntax
      self.httpSyntax = data.find(function(element) {
        return element.name === "HTTP";
      });

      /**
       * Retrieve all service instances
       */
      self.ServiceInstance.init().then(function() {
        // setting all view services in cache
        self.filteredServices = self.ServiceInstance.list({'service_type_id': self.ServiceInstance.types.VIEW});
        /**
         * Retrieve all data series
         */
        return DataSeriesService.init({schema: "all"}).then(function(dataSeries) {
          self.dataSeries = dataSeries;

          var styleCache = config.view.style;

          if (self.view.data_series_id) {
            self.onDataSeriesChanged(self.view.data_series_id);

            switch(self.viewDataSeries.data_series_semantics.data_series_type_name) {
              case self.DataSeriesType.GRID:
                self.view.style = styleCache;
                // -------------------------------------------------------------------------------
                // TODO: It is temp. It should have angular xml parser or a library to extend app
                // -------------------------------------------------------------------------------
                var xmlStyle = $(self.view.style || "");
                var colors = $(self.view.style || "").find("ColorMapEntry");
                if (colors.length !== 0) {
                  var minColorSelector = $(colors[0]);
                  var maxColorSelector = $(colors[1]);
                  self.minColor = minColorSelector.attr("color");
                  self.minValue = parseInt(minColorSelector.attr("quantity"));

                  self.maxColor = maxColorSelector.attr("color");
                  self.maxValue = parseInt(maxColorSelector.attr("quantity"));
                }
                break;
              case self.DataSeriesType.GEOMETRIC_OBJECT:
                var legend = config.view.legend;
                self.view.typeId = legend.type_id;
                self.view.bands = legend.bands || 5;
                self.view.column = legend.column;
                self.colors = legend.colors;
                self.handleColor();
                break;
            }
          }
          /**
           * Configuring Schema form http. This sentence is important because child controller may be not initialized yet.
           * Using $timeout 0 forces to execute when angular ready state is OK.
           */
          $timeout(function() {
            $scope.schema = {
              type: "object",
              properties: self.httpSyntax.properties,
              required: self.httpSyntax.required || []
            };

            if (self.isUpdating) {
              self.schedule = {};
              $scope.$broadcast("updateSchedule", self.view.schedule || {});
            } else {
              if (!config.view) {
                // forcing first view pre-selected
                if (self.filteredServices.length > 0) {
                  self.view.service_instance_id = self.filteredServices[0].id;
                }
              }
            }

            $scope.form = self.httpSyntax.display ? self.httpSyntax.display : ["*"];

            $scope.$broadcast('schemaFormRedraw');
          });
        });
      });
    }).catch(function(err) {
      $log.info(err);
      self.MessageBoxService.danger(i18n.__("View"), err);
    });

    // Setting view service dao
    self.ViewService = ViewService;
    /**
     * It is used on ng-init active view. It will wait for angular ready condition and set active view checkbox
     * 
     * @returns {void}
     */
    self.initActive = function() {
      // wait angular digest cycle
      $timeout(function() {
        self.view.active = (config.view.active === false || config.view.active) ? config.view.active : true;
      });
    };

    self.populateColumns = function(type) {
      if (type === undefined || type === "") {
        self.columns = [];
        return;
      }

      FieldRetriever.retrieve({type: "dataseries", format: type})
        .then(function(columns) {
          self.columns = columns;
        });
    };

    /**
     * It handles Data Series combobox change. If it is GRID data series, there is a default style script
     * @param {DataSeries}
     */
    self.onDataSeriesChanged = function(dataSeriesId) {
      self.dataSeries.some(function(dSeries) {
        if (dSeries.id === dataSeriesId) {
          // setting view data series
          self.viewDataSeries = dSeries;
          // extra comparison just to setting if it is dynamic or static.
          // Here avoids to setting to true in many cases below
          self.isDynamic = dSeries.data_series_semantics.data_series_type_name !== DataSeriesService.DataSeriesType.GEOMETRIC_OBJECT;

          // tries to populate columns
          // self.populateColumns(dSeries.data_series_semantics.data_format_name);

          self.view.style = makeStyle(dSeries.data_series_semantics.data_series_type_name);
          // breaking loop
          return true;
        }
      });
    }
    /**
     * It contains all forms. It must be appended on scope instance due schema form support;
     * @type {Object}
     */
    $scope.forms = {};
    /**
     * It represents a terrama2 box styles
     * @type {Object}
     */
    self.css = {
      boxType: "box-solid"
    };
    /**
     * It contains view instance values
     * @type {Object}
     */
    self.view = config.view || {};
    // setting default band value
    self.view.bands = 5;
    /**
     * It defines a selected View DataSeries object
     * @type {DataSeries}
     */
    self.viewDataSeries = {};
    /**
     * Helper to reset alert box instance
     */
    self.close = function() {
      self.MessageBoxService.reset();
    };
    /**
     * It handles file upload to retrieve xml style
     */
    self.onFileUploadClick = function() {
      
    };
    /**
     * It performs a save operation. It applies a form validation and try to save
     * @param {boolean} shouldRun - Determines if service should auto-run after save process
     * @returns {void}
     */
    self.save = function(shouldRun) {
      // broadcasting each one terrama2 field directive validation 
      $scope.$broadcast("formFieldValidation");
      // broadcasting schema form validation
      $scope.$broadcast("schemaFormValidate");

      $timeout(function(){
        $scope.$apply(function() {
          if ($scope.forms.viewForm.$invalid || 
            $scope.forms.dataSeriesForm.$invalid ||
            $scope.forms.styleForm.$invalid) {
            return;
          }
          var sld = makeStyle(self.viewDataSeries.data_series_semantics.data_series_type_name);
          // setting style
          switch(self.viewDataSeries.data_series_semantics.data_series_type_name) {
            case self.DataSeriesType.GRID:
              // digesting XML with min/max value and color
              self.view.style = StringFormat(sld, self.view.name, self.minColor, self.minValue, self.maxColor, self.maxValue);
              break;
            default:
              self.view.style = "";
              self.view.colors = self.colors;
          }

          // If dynamic, schedule validation is required
          if (self.isDynamic) {
            /**
             * @todo Implement Angular ScheduleService to handle it, since is common on dynamic data series and analysis registration.
             */
            var scheduleForm = angular.element('form[name="scheduleForm"]').scope()['scheduleForm'];
            // form validation
            if (scheduleForm.$invalid) {
              return;
            }

            // preparing schedule.  
            var scheduleValues = self.view.schedule;
            switch(scheduleValues.scheduleHandler) {
              case "seconds":
              case "minutes":
              case "hours":
                scheduleValues.frequency_unit = scheduleValues.scheduleHandler;
                scheduleValues.frequency_start_time = scheduleValues.frequency_start_time ? scheduleValues.frequency_start_time.toISOString() : "";
                break;
              case "weeks":
              case "monthly":
              case "yearly":
                // todo: verify
                var dt = scheduleValues.schedule_time;
                scheduleValues.schedule_unit = scheduleValues.scheduleHandler;
                scheduleValues.schedule_time = moment(dt).format("HH:mm:ss");
                break;

              default:
                break;
            }
          } // end if isDynamic

          /**
           * It contains a view model with flag "run" to determines if service should run
           * @type {Object}
           */
          var mergedView = angular.merge(self.view, {run: shouldRun});
          // tries to save
          var operation = self.isUpdating ? self.ViewService.update(self.view.id, self.view) : self.ViewService.create(self.view);
          operation.then(function(response) {
            $log.info(response);
            $window.location.href = "/configuration/views?token=" + response.token
          }).catch(function(err) {
            $log.info(err);
            self.MessageBoxService.danger(i18n.__("View"), err);
          });
        });
      });
    };
  }

  ViewRegisterUpdate.$inject = ["$scope", "i18n", "ViewService", "$log", "$http", "$timeout", "MessageBoxService", "$window", 
    "DataSeriesService", "Service", "StyleConstants", "StringFormat", "StyleType", "ColorFactory", "FieldRetriever"];

  return ViewRegisterUpdate;
});