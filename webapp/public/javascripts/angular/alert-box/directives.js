angular.module("terrama2.components.messagebox", [])
  .directive("terrama2MessageBox", function() {
    return {
      restrict: "AE",
      replace: true,
      transclude: true,
      templateUrl: "/javascripts/angular/alert-box/templates/alert-box.html",
      scope: {
        errorFound: "=errorFound",
        message: "=",
        title: "=",
        close: "&"
      },
    }
  })

  .directive("terrama2AlertBox", function() {
    return {
      restrict: "E",
      templateUrl: "/javascripts/angular/alert-box/templates/message-box.html",
      scope: {
        alertLevel: '=alertLevel',
        title: "=title",
        message: "=message",
        close: "&?close",
        display: "=displayHandler",
        extra: '=?extra'
      },
      controller: function($scope) {
        const ALERT_LEVELS = {
          INFO: "info",
          WARNING: 'warning',
          DANGER: 'danger',
          SUCCESS: 'success'
        };

        // setting default close function
        if (!$scope.close) {
          $scope.close = function() {
            $scope.display = false;
          }
        }

        $scope.$watch("display", function(val) {
          console.log(val);
        });

        $scope.isAnyExtra = function() {
          return Object.keys($scope.extra || {}).length > 0;
        }

        $scope.alertIcon = function() {
          switch($scope.alertLevel) {
            case ALERT_LEVELS.INFO:
              return "fa-info";
              break;
            case ALERT_LEVELS.WARNING:
              return "fa-exclamation-triangle";
              break;
            case ALERT_LEVELS.DANGER:
              return "fa-times";
              break;
            case ALERT_LEVELS.SUCCESS:
              return "fa-check";
              break;
            default:
              return "fa-info";
          }
        }
      }
    }
  })

  .directive("terrama2ModalBox", function() {
    return {
      restrict: 'E',
      transclude: true,
      templateUrl: '/javascripts/angular/alert-box/templates/modal.html',
      scope: {
        title: '=title',
        modalId: '=modalId',
        modalType: '=modalType',
        properties: '=?properties'
      },
      controller: function($scope) {
        if ($scope.properties === undefined)
          $scope.properties = {};
        $scope.css = $scope.properties.css || "modal fade"
      }
    }
  });
