define(function() {
  /**
   * It defines a TerraMA² View Service DAO.
   * 
   * @class ViewService
   * 
   * @param {BaseService} BaseService - Angular TerraMA² base service module
   * @param {angular.IQ} $q - Angular promiser module 
   */
  function ViewService(BaseService, $q) {
    var self = this;
    this.BaseService = BaseService;
    this.$q = $q;
    this.$baseUrl = "/api/View";
    this.model = [];

    /**
     * It retrieves all data series semantics and cache them in model.
     * 
     * @param {Object} restriction
     * @returns {angular.IPromise<Object[]>}
     */
    this.init = function(restriction) {
      var defer = self.BaseService.$q.defer();

      self.BaseService
        .$request(self.$baseUrl, "GET", {params: restriction})
        .then(function(data) {
          self.model = data;
          return defer.resolve(data);
        })
        .catch(function(err) {
          return defer.reject(err);
        })

      return defer.promise;
    };

    /**
     * It performs a view creation on API call and stores in cache
     * 
     * @param {Object} viewObject - A view values
     * @returns {ng.IPromise}
     */
    this.create = function(viewObject) {
      var defer = self.$q.defer();
      self.BaseService.$request(self.$baseUrl, "POST", {
        data: viewObject,
        headers: {
          "Content-Type": "application/json"
        }
      })
      .then(function(newView) {
        self.model.push(newView);
        return defer.resolve(newView);
      })
      .catch(function(err) {
        return defer.reject(err);
      })

      return defer.promise;
    };

    /**
     * It performs a view creation on API call.
     * 
     * @param {Object} viewObject - A view values
     * @returns {ng.IPromise}
     */
    this.list = function(restriction) {
      return self.BaseService.$filter('filter')(self.model, restriction);
    };
    
    /**
     * It performs a view creation on API call.
     * 
     * @param {number} viewId - A view identifier
     * @param {Object} viewObject - A view values
     * @returns {ng.IPromise}
     */
    this.get = function(viewId, restriction) {
      return self.BaseService.get(self.model, restriction);
    };

    /**
     * It performs a view update on API call
     * 
     * @param {number} viewId - A view identifier
     * @param {Object} viewObject - A view values to update
     * @returns {ng.IPromise}
     */
    this.update = function(viewId, viewObject) {
      return self.BaseService.$request(self.$baseUrl + "/" + viewId, "PUT", {
        data: viewObject
      });
    };
  } // end ViewService

  ViewService.$inject = ["BaseService", "$q"];

  return ViewService;
} ());