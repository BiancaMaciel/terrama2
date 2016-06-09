angular.module("terrama2.schedule", ['terrama2'])
  .directive("terrama2Schedule", function() {
    return {
      restrict: 'E',
      templateUrl: '/javascripts/angular/data-series/templates/schedule.html',
      scope: {
        model: '=model',
        options: '=?options'
      },
      controller: ['$scope', 'i18n', function($scope, i18n) {
        $scope.i18n = i18n;
        $scope.isFrequency = false;
        $scope.isSchedule = false;

        $scope.weekDays = [
          {id: 1, name: i18n.__('Sunday')},
          {id: 2, name: i18n.__('Monday')},
          {id: 3, name: i18n.__('Tuesday')},
          {id: 4, name: i18n.__('Wednesday')},
          {id: 5, name: i18n.__('Thursday')},
          {id: 6, name: i18n.__('Friday')},
          {id: 7, name: i18n.__('Saturday')},
        ];

        $scope.tryParseInt = function(value) {
          if (isNaN(value))
            return value;
          return parseInt(value);
        }

        $scope.$on("updateSchedule", function(event, scheduleObject) {
          if (scheduleObject.schedule_unit) {
            $scope.model.scheduleHandler = scheduleObject.schedule_unit;
            $scope.model.schedule = scheduleObject.schedule;
            $scope.model.schedule_time = scheduleObject.schedule_time;
          }else if (scheduleObject.frequency_unit) {
            $scope.model.frequency = scheduleObject.frequency;
            $scope.model.frequency_unit = scheduleObject.frequency_unit;
            $scope.model.scheduleHandler = scheduleObject.frequency_unit;
          }

          $scope.onScheduleChange($scope.model.scheduleHandler)
        });

        $scope.onScheduleChange = function(value) {
          var resetHelper = function(i) {
            if (i == 1) {
              delete $scope.model.schedule;
              delete $scope.model.schedule_retry;
              delete $scope.model.schedule_retry_unit;
              delete $scope.model.schedule_timeout;
              delete $scope.model.schedule_timeout_unit;
              $scope.isFrequency = true;
              $scope.isSchedule = false;
            } else if (i == 2) {
              delete $scope.model.frequency;
              delete $scope.model.frequency_unit;
              $scope.isFrequency = false;
              $scope.isSchedule = true;
            }
          };

          switch(value) {
            case "seconds":
            case "minutes":
            case "hours":
              resetHelper(1);
              $scope.minSchedule = 0;
              $scope.maxSchedule = 2147483648; // setting max value to schedule (int32)
              break;
            case "weeks":
              resetHelper(2);
              $scope.minSchedule = 1;
              $scope.maxSchedule = 7;
              break;
            case "monthly":
              resetHelper(2);
              $scope.minSchedule = 1;
              $scope.maxSchedule = 31;
              break;
            case "yearly":
              resetHelper(2);
              $scope.minSchedule = 1;
              $scope.maxSchedule = 366;
              break;
            default:
              $scope.minSchedule = 0;
              $scope.maxSchedule = 0;
              break;
          }
        };
      }]
    }
  });
