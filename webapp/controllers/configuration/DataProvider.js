var DataManager = require("../../core/DataManager");
var Utils = require('../../helpers/Utils');
var makeTokenParameters = require('../../core/Utils').makeTokenParameters;
var UriBuilder = require('../../core/UriBuilder');
var RequestFactory = require("../../core/RequestFactory");


module.exports = function(app) {
  return {
    get: function(request, response) {
      var dataProviderId = request.query.id,
          method = request.query.method;

      var parameters = makeTokenParameters(request.query.token, app);

      return response.render("configuration/providers", parameters);
    },

    new: function(request, response) {
      var redirectTo = request.query.redirectTo ? request.query : {redirectTo: "/configuration/providers"};

      return response.render("configuration/provider", {
        isEditing: false,
        dataProvider: {},
        saveConfig: {
          url: "/api/DataProvider",
          method: "POST"
        },
        redirectTo: redirectTo
      });
    },

    edit: function(request, response) {
      var dataProviderName = request.params.name;
      var redirectTo = request.query.redirectTo ? request.query : {redirectTo: "/configuration/providers"};

      DataManager.getDataProvider({name: dataProviderName}).then(function(dataProvider) {
        var requester = RequestFactory.buildFromUri(dataProvider.uri);

        return response.render('configuration/provider', {
          isEditing: true,
          dataProvider: {
            name: dataProvider.name,
            description: dataProvider.description,
            active: dataProvider.active,
            data_provider_type_name: dataProvider.data_provider_type_name,
            uriObject: requester.params
          },
          saveConfig: {
            url: "/api/DataProvider/" + dataProvider.name,
            method: "PUT"
          },
          fields: requester.constructor.fields(),
          redirectTo: redirectTo
        });
      }).catch(function(err) {
        console.log(err);
        response.render("base/404");
      });
    }
  };
};