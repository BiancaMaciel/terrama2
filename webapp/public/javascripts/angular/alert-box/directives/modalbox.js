define(function() {
  function terrama2ModalBox() {
    return {
      restrict: 'E',
      transclude: true,
      templateUrl: '/javascripts/angular/alert-box/templates/modal.html',
      scope: {
        title: '=title',
        modalId: '=modalId',
        // klass: "=class",
        modalType: '=modalType',
        properties: '=?properties'
      },
      controller: ["$scope", function($scope) {
        if ($scope.properties === undefined) { $scope.properties = {}; }
        $scope.css = $scope.properties.css || "modal fade";
      }]
    };
  }

  return terrama2ModalBox;
})