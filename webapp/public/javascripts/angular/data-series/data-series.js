angular.module('terrama2.listDataSeries', ['terrama2.table', 'terrama2.services', 'terrama2.components.messagebox', 'terrama2.administration.services.iservices'])
  .controller("ListController", ['$scope', 'DataSeriesFactory', 'Socket', 'i18n', '$window', 'Service',
  function($scope, DataSeriesFactory,Socket, i18n, $window, Service) {
    $scope.i18n = i18n;
    $scope.disabledButtons = {};
    var isDynamic = false;
    var queryParams = {};

    var serviceCache = {};

    var config = $window.configuration;

    Service.init();

    Socket.on('errorResponse', function(response){
      $scope.display = true;
      $scope.alertLevel = "alert-danger";
      $scope.alertBox.message = response.message;
    });

    Socket.on('runResponse', function(response){
      $scope.display = true;
      $scope.alertLevel = "alert-success";
      $scope.alertBox.message = i18n.__("The process was started successfully");
    });

    Socket.on('statusResponse', function(response) {
      if(response.checking === undefined) {
        if(response.online) {
          Socket.emit('run', serviceCache[response.service].process_ids);
        } else {
          var service = Service.get(serviceCache[response.service].process_ids.service_instance);

          $scope.display = true;
          $scope.alertLevel = "alert-error";

          if(service != null) {
            $scope.alertBox.message = i18n.__("Service") + " '" + service.name + "' " + i18n.__("is not active");
          } else {
            $scope.alertBox.message = i18n.__("Service not active");
          }
        }

        delete $scope.disabledButtons[serviceCache[response.service].service_id];
        delete serviceCache[response.service];
      }
    });

    if (config.dataSeriesType == "static") {
      $scope.dataSeriesType = config.dataSeriesType;
    } else {
      $scope.dataSeriesType = 'dynamic';
      queryParams['collector'] = true;
    }

    queryParams['type'] = $scope.dataSeriesType;

    $scope.model = [];
    $scope.fields = [];

    $scope.remove = function(object) {
      return "/api/DataSeries/" + object.id + "/delete";
    };

    $scope.extra = {
      removeOperationCallback: function(err, data) {
        $scope.display = true;
        if (err) {
          $scope.alertLevel = "alert-danger";
          $scope.alertBox.message = err.message;
          return;
        }

        $scope.alertLevel = "alert-success";
        $scope.alertBox.message = data.name + i18n.__(" removed");
      },
      showRunButton: config.showRunButton,
      canRun: function(object){
        var foundCollector = config.collectors.find(function(collector){
          return collector.output_data_series == object.id;
        });
        var foundAnalysis = config.analysis.find(function(analysi){
          return analysi.dataSeries.id == object.id;
        })
        return foundCollector || foundAnalysis;
      },
      run: function(object){
        var service_instance = this.canRun(object);

        serviceCache[service_instance.service_instance_id] = {
          "process_ids": {
            "ids": [service_instance.id],
            "service_instance": service_instance.service_instance_id
          },
          "service_id": object.id,
          "service_name": object.name
        };

        $scope.disabledButtons[object.id] = true;

        Socket.emit('status', {service: service_instance.service_instance_id});
      },
      disabledButtons: function(object){
        return $scope.disabledButtons[object.id];
      }
    };
    $scope.method = "{[ method ]}";
    $scope.alertLevel = "alert-success";
    $scope.alertBox = {
      title: i18n.__("Data Series"),
      message: config.message
    };
    $scope.resetState = function() { $scope.display = false; };
    $scope.display = config.message !== "";

    DataSeriesFactory.get(queryParams).success(function(data) {
      $scope.model = data instanceof Array ? data : [];

      // processing type
      $scope.model.forEach(function(instance) {
        var value;
        switch(instance.data_series_semantics.data_series_type_name) {
          case globals.enums.DataSeriesType.DCP:
            value = i18n.__("DCP");
            break;
          case globals.enums.DataSeriesType.ANALYSIS_MONITORED_OBJECT:
          case globals.enums.DataSeriesType.OCCURRENCE:
            value = i18n.__("Occurrence");
            break;
          case globals.enums.DataSeriesType.GRID:
            value = i18n.__("Grid");
            break;
          case globals.enums.DataSeriesType.GEOMETRIC_OBJECT:
            value = i18n.__("Geometric Object");
            break;
          default:
            value = instance.data_series_semantics.name;
        }

        instance.model_type = value;
      });
      $scope.fields = [{key: 'name', as: i18n.__("Name")}, {key: "model_type", as: i18n.__("Type")}];
    });

    $scope.link = config.link || null;

    $scope.linkToAdd = config.linkToAdd || null;

    $scope.iconFn = config.iconFn || null;

    $scope.iconProperties = config.iconProperties || {};
  }]);
