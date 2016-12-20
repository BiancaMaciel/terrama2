angular.module("terrama2.analysis", [
    "terrama2",
    "terrama2.services",
    "terrama2.table",
    "terrama2.components.messagebox",
    "terrama2.components.messagebox.services",
    "terrama2.analysis.services",
    "terrama2.administration.services.iservices"
  ])
  .controller("ListController", [
    "$scope",
    "$http",
    "AnalysisService",
    "MessageBoxService",
    "Socket",
    "i18n",
    "$log",
    "Service",
    "$window",
    function($scope, $http, AnalysisService, MessageBoxService, Socket, i18n, $log, Service, $window) {
      var config = $window.configuration;
      var globals = $window.globals;
      $scope.model = [];
      $scope.i18n = i18n;
      $scope.disabledButtons = {};

      var serviceCache = {};

      Service.init();
      var title = i18n.__("Analysis");

      Socket.on('errorResponse', function(response){
        MessageBoxService.danger(title, response.message);
      });

      Socket.on('runResponse', function(response){
        MessageBoxService.danger(title, i18n.__("The process was started successfully"));
      })

      Socket.on('statusResponse', function(response) {
        if(response.checking === undefined || (!response.checking && response.status == 400)) {
          if(response.online) {
            Socket.emit('run', serviceCache[response.service].process_ids);
          } else {
            var errMessage = "";
            if(serviceCache[response.service] != undefined) {
              var service = Service.get(serviceCache[response.service].process_ids.service_instance);

              if(service != null) {
                errMessage = i18n.__("Service") + " '" + service.name + "' " + i18n.__("is not active");
              } else {
                errMessage = i18n.__("Service not active");
              }
            } else {
              errMessage = i18n.__("Service not active");
            }

            MessageBoxService.danger(title, errMessage);
          }

          delete $scope.disabledButtons[serviceCache[response.service].service_id];
          delete serviceCache[response.service];
        }
      });

      var restriction = {
        project_id: config.projectId
      };

      $scope.iconProperties = {
        type: "img"
      };

      $scope.fields = [
        {key: "name", as: i18n.__("Name")},
        {key: "type.name", as: i18n.__("Type")}
      ];

      $scope.linkToAdd = "/configuration/analysis/new";

      $scope.link = function(object) {
        return "/configuration/analysis/" + object.id + "/edit";
      };

      if (config.message !== "") {
        MessageBoxService.success(title, config.message);
      }

      $scope.extra = {
        removeOperationCallback: function(err, data) {
          if (err) {
            MessageBoxService.danger(title, err.message);
            return;
          }
          MessageBoxService.success(title, data.name + " removed");
        },
        showRunButton: true,
        canRun: function(object){
          return true;
        },
        run: function(object){
          serviceCache[object.service_instance_id] = {
            "process_ids": {
              "ids":[object.id],
              "service_instance": object.service_instance_id
            },
            "service_id": object.id,
            "service_name": object.name
          };

          $scope.disabledButtons[object.id] = true;

          Socket.emit('status', {service: object.service_instance_id});
        },
        disabledButtons: function(object){
          return $scope.disabledButtons[object.id];
        }
      };

      $scope.remove = function(object) {
        return "/api/Analysis/" + object.id + "/delete";
      };

      $scope.icon = function(object) {
        switch(object.type.id) {
          case globals.enums.AnalysisType.MONITORED:
            return "/images/monitored-object.png";
          case globals.enums.AnalysisType.GRID:
            return "/images/grid-small.png";
          default:
            return "/images/map-pin.png";
        }
      };

      AnalysisService.init(restriction)
        .then(function(analysis) {
          $scope.model = analysis;
        }).catch(function(err) {
          $log.info("Err in retrieving Analysis " + err);
        }).finally(function() {
          $scope.loading = false;
        });
    }]);