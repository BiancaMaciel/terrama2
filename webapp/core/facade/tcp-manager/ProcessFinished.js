(function() {
  'use strict';
  /**
   * TerraMA² DataManager module
   * @type {DataManager}
   */
  var DataManager = require("./../../DataManager");
  var Utils = require("./../../Utils");
  var PromiseClass = require("bluebird");
  var ServiceTypeError = require("./../../Exceptions").ServiceTypeError;
  var ServiceType = require("./../../Enums").ServiceType;

  /**
   * It represents a mock to handle ProcessFinished events, inserting/updating object retrieved from C++ services
   * 
   * @class ProcessFinished
   */
  var ProcessFinished = module.exports = {};
  /**
   * It handles process finished response from C++ service.
   * 
   * @param {Object} response - A registered object retrieved from C++ services.
   * @param {number} instance_id - A TerraMA² service instance identifier
   * @param {number} process_id - A TerraMA² process executed. The process may be: View, Analysis or Collector.
   * @param {?} response.any - A response values retrieved from C++ services
   * @returns {Promise<?>} 
   */
  ProcessFinished.handle = function(response) {
    var self = this;
    return new PromiseClass(function(resolve, reject) {
      var handler = null;
      // retrieving service instance
      return DataManager.getServiceInstance({id: response.instance_id})
        .then(function(service) {
          switch(service.service_type_id) {
            case ServiceType.ANALYSIS:
            case ServiceType.COLLECTOR:
              throw new ServiceTypeError(Utils.format(
                "Analysis and Collector process finished is not implemented yet %s", response.instance_id));
            case ServiceType.VIEW:
              handler = self.handleRegisteredViews(response);
              break;
            default:
              throw new ServiceTypeError(Utils.format("Invalid instance id %s", response.instance_id));
          }

          return handler
            .then(function(handlerResult) {
              return resolve(handlerResult);
            })
            // on Error
            .catch(function(err) {
              return reject(err);
            });
        })
        // on any error
        .catch(function() {
          return reject(err);
        });
    });
  };
  /**
   * It handles registered views process finished. Once values received, it tries to retrieve a registered view
   * by view_id. If found, applies upSert layers operation (insert or update). If not found, add a new registered view
   * 
   * @param {Object} registeredViewObject - A registered object retrieved from C++ services.
   * @param {string} class - A class name
   * @param {number} process_id - Determines which view were processed
   * 
   * @returns {Promise<RegisteredView>}
   */
  ProcessFinished.handleRegisteredViews = function(registeredViewObject) {
    return new PromiseClass(function(resolve, reject) {
      // preparing transaction mode
      return DataManager.orm(function(t) {
        var options = {transaction: t};

        return DataManager.getRegisteredView({view_id: registeredViewObject.process_id}, options)
          // on registeredView retrieved, performs update layers
          .then(function(registeredView) {
            var promises = [];
            registeredViewObject.layers_list.forEach(function(layer) {
              promises.push(DataManager.upsertLayer(registeredView.id, {name: layer}, options));
            });
            return Promise.all(promises)
              .then(function() {
                return DataManager.getRegisteredView({view_id: registeredViewObject.process_id}, options)
              });
          })
          // NotFound... tries to insert a new one
          .catch(function(err) {
            return DataManager.addRegisteredView(registeredViewObject, options);
          });
      })
      // on commit
      .then(function(registeredView) {
        return resolve(registeredView);
      })
      // on any error
      .catch(function(err) {
        return reject(new Error(Utils.format("Error during registered views: %s" + err.toString())));
      });
    });
  };

} ());